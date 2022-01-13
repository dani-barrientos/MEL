#include <core/Thread.h>
using core::Thread;
#include <tasking/Runnable.h>
using tasking::Runnable;
using ::tasking::_private::RTMemPool;
using ::tasking::_private::RTMemBlock;
using ::tasking::_private::MemZoneList;


#include <core/Timer.h>
using core::Timer;
#include <mpl/LinkArgs.h>
using mpl::linkFunctor;


#include <mpl/deleter.h>
#include <algorithm>
using std::for_each;

#include <core/TLS.h>
using core::TLS;
static TLS::TLSKey gCurrentRunnableKey;
static bool gCurrentRunnableKeyCreated = false;
static CriticalSection gCurrrentRunnableCS;

void* ::tasking::_private::RunnableTask::operator new( size_t s,Runnable* owner ) 
{
	return ::operator new(s);
	/*
	descartado hasta ahcer sistema eficiente
	Lock lck(owner->mMemPoolCS);
	//std::lock_guard<std::mutex> lck(owner->mMemPoolCS);
	RTMemPool* selectedPool = NULL;
	//run over memory zones looking for free one
	for( auto& i:owner->mRTZone.getList())
	{
		if ( i.count < owner->mMaxTaskSize )
		{
			selectedPool = &i;
			break;
		}
	}

	
	if ( !selectedPool ) //no free pool, create new
	{
		spdlog::warn("New pool needed for Runnable!!");
		selectedPool = owner->_addNewPool();
	}
	if ( selectedPool )
	{
		//find first free block
		auto pool = selectedPool->pool;
		for ( unsigned int i = 0; i < owner->mMaxTaskSize; ++i )
		{
			if ( pool[i].memState == RTMemBlock::EMemState::FREE )
			{
				pool[i].memState = RTMemBlock::EMemState::USED;
				++selectedPool->count;
				return &pool[i].task;
			}
		}
	}
	//Logger::getLogger()->fatal( "Runnable::not enough memory" );
	throw std::bad_alloc();
	*/
}

void ::tasking::_private::RunnableTask::operator delete( void* ptr ) noexcept
{
	::operator delete(ptr);
	/*
	Runnable* ownerRunnable;
	RTMemBlock* block = (RTMemBlock*)((char*)ptr - offsetof( RTMemBlock, task));
	ownerRunnable = block->owner->owner;
	Lock lck(ownerRunnable->mMemPoolCS);
	//std::lock_guard<std::mutex> lck(ownerRunnable->mMemPoolCS);
	block->memState = RTMemBlock::EMemState::FREE;
	if (--block->owner->count == 0 )
	{
		//remove pool only if it's not the first pool
		if ( ownerRunnable->mRTZone.size() > 1 )
			ownerRunnable->_removePool( block->owner );
	}
	*/
}
DABAL_CORE_OBJECT_TYPEINFO_IMPL_ROOT(Runnable);

RTMemPool* Runnable::_addNewPool()
{
	RTMemPool* result;
	RTMemPool auxPool;
	//reserve memory and mark all blocks as free 
	auxPool.owner = this;
	auxPool.pool = (RTMemBlock*)malloc( sizeof(RTMemBlock)*mMaxTaskSize );
	auxPool.count = 0;
	mRTZone.push_front(std::move(auxPool));
	//@todo guardar iterador
	auto poolIterator = mRTZone.getList().begin();
	//poolIterator->iterator = poolIterator;
	result = &(*poolIterator);
	for ( unsigned int i = 0; i < mMaxTaskSize; ++i )
	{
		result->pool[i].memState = RTMemBlock::EMemState::FREE;
		result->pool[i].owner = result;
	}
	return result;
}
void Runnable::_removePool( RTMemPool* pool )
{
	//store pool in auxiliar variable
	RTMemBlock* memZone = pool->pool;
	//remove from list
	mRTZone.remove(pool);
	//mRTZone.erase( pool->iterator );
	//free memory
	free(memZone); 
}
Runnable::RunnableInfo* Runnable::_getCurrentRunnableInfo()
{
	if (gCurrentRunnableKeyCreated) //not multithread-safe but it shouldn't be a problem
	{
		auto ri = (RunnableInfo*)TLS::getValue(gCurrentRunnableKey);
		return ri;
	}
	else
	{
		return nullptr;
	}
}
Runnable* Runnable::getCurrentRunnable()
{
	auto ri = _getCurrentRunnableInfo();
	if (ri) //not multithread-safe but it shouldn't be a problem
	{
		return ri->current;
	}
	else
	{
		return nullptr;
	}
	/*if (gCurrentRunnableKeyCreated) //not multithread-safe but it shouldn't be a problem
	{
		return (Runnable*)TLS::getValue(gCurrentRunnableKey);
	}
	else
	{
		return NULL;
	}*/
}

Runnable::Runnable(unsigned int maxTaskSize):
	mMaxTaskSize(maxTaskSize),
	mOwnerThread(0),  //�assume 0 is invalid thread id!!
	mCurrentInfo(nullptr)
{
		
	//create one default pool
	gCurrrentRunnableCS.enter();
	
	
	if (!gCurrentRunnableKeyCreated)
	{
		TLS::createKey(gCurrentRunnableKey);		
		gCurrentRunnableKeyCreated = true;
	}
	
	gCurrrentRunnableCS.leave();
	_addNewPool();
}

Runnable::~Runnable() {
	mTasks.destroyAllProcesses();
	for( auto& i:mRTZone.getList())
	{
		free(i.pool);
		
	}
	
}



unsigned int Runnable::run()
{
	//now initialize the value
	RunnableInfo* ri = _getCurrentRunnableInfo();
	if (ri == NULL)
	{
		ri = new RunnableInfo; //@todo ahora quedar� esta perdida de memoria
		TLS::setValue(gCurrentRunnableKey, ri);
	}
	mCurrentInfo = ri;
	Runnable* current = nullptr;
	ri->current = this;
	unsigned int result;
	if (getTimer() == NULL)
		setTimer(std::make_shared<Timer>());
	mOwnerThread = ::core::getCurrentThreadId();
	result = onRun();
	executeFinishEvents();
	ri->current = current;
	/*Runnable* current = getCurrentRunnable();
	TLS::setValue(gCurrentRunnableKey, this);	
	unsigned int result;
	if ( getTimer() == NULL  )
		setTimer( new Timer() );
	mOwnerThread = ::core::getCurrentThreadId();
	result = onRun();
	executeFinishEvents();
	TLS::setValue(gCurrentRunnableKey, current);  //Restore previous
	*/
	return result;
}
void Runnable::setTimer(std::shared_ptr<Timer> timer )
{
	mTasks.setTimer( timer );
}

void Runnable::postTask(std::shared_ptr<Process> process, unsigned int startTime)
{
	assert( process && "is NULL");
	mTasks.insertProcess( process,startTime );
	onPostTask( process );
}


void Runnable::processTasks()
{
	Runnable* oldR = mCurrentInfo->current;
	mCurrentInfo->current = this;
	mTasks.executeProcesses();
	mCurrentInfo->current = oldR;
	//RunnableInfo* current = getCurrentRunnableInfo();
	//if ( current != this )
	//	TLS::setValue(gCurrentRunnableKey, this);
	//mTasks.executeProcesses();
	//if (current != this)
	//	TLS::setValue(gCurrentRunnableKey, current);  //Restore previous
}


//#pragma optimize("",off)


void Runnable::executeFinishEvents()
{
	mFinishEvents.triggerCallbacks(this);
	/*for( list< TFinishEvent* >::iterator i = mFinishEvents.begin() ; i != mFinishEvents.end(); ++i )
	{
		(**i)( this );
	}*/
}

void Runnable::_triggerOnDone( const ::core::Future_Base& future, Callback<void,const ::core::Future_Base&>* cb,
	FutureTriggerInfo* info )
{
	if ( !info->getCancel() )
	{
		core::FutureData_Base::EWaitResult waitResult = future.waitAsMThread();
		if ( !info->getCancel() && 
			waitResult != ::core::FutureData_Base::FUTURE_RECEIVED_KILL_SIGNAL )
			(*cb)( future );
		/*if ( !info->getCancel() &&
			(!future.getError() || future.getError()->error != FutureData_Base::FUTURE_RECEIVED_KILL_SIGNAL))
		{
			(*cb)( future );
		}*/
	}
	delete cb;
	delete info;
}
void Runnable::_sleep( unsigned int msegs )
{
	Thread::sleep( msegs );
}
//#pragma optimize("",on)


Runnable::FutureTriggerInfo* Runnable::triggerOnDone(const ::core::Future_Base& future, std::function<void(const ::core::Future_Base&)>&& f, bool autoKill,void* extraInfo) {
    if ( !future.getValid() )
    {
        FutureTriggerInfo* info = new FutureTriggerInfo;
        typedef Callback<void,const ::core::Future_Base&> TCallback;
        TCallback* cb = new TCallback( f, ::core::use_function );
        post(
             RUNNABLE_CREATETASK(
                returnAdaptor<void>
                (
                linkFunctor<void,TYPELIST()>( makeMemberEncapsulate( &Runnable::_triggerOnDone, this ),future,cb,info)
                ,::tasking::EGenericProcessResult::KILL
                )
                ),autoKill/* , ::core::Runnable::NORMAL_PRIORITY_TASK */, 0, 0/*, extraInfo*/
             );
        return info;
    }else
    {
        f( future );
        return NULL;
    }
}
