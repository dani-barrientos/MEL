#include <core/Thread.h>
using mel::core::Thread;
#include <tasking/Runnable.h>
using mel::tasking::Runnable;
using ::mel::tasking::_private::RTMemPool;
using ::mel::tasking::_private::RTMemBlock;
using ::mel::tasking::_private::MemZoneList;
using ::mel::tasking::ProcessFactory;

#include <core/Timer.h>
using mel::core::Timer;
#include <mpl/LinkArgs.h>
using mel::mpl::linkFunctor;


#include <mpl/deleter.h>
#include <algorithm>
using std::for_each;

#include <core/TLS.h>
using mel::core::TLS;
static TLS::TLSKey gCurrentRunnableKey;
static bool gCurrentRunnableKeyCreated = false;
static std::mutex gCurrrentRunnableCS;

GenericProcess* mel::tasking::DefaultAllocator::allocate(Runnable* _this)
{
	return _this->getDefaultFactory()->create(_this);
}
GenericProcess* ProcessFactory::onCreate(Runnable* owner) const
{
	return new (owner)::mel::tasking::_private::RunnableTask();
}
void* ::mel::tasking::_private::RunnableTask::operator new( size_t s,Runnable* owner ) 
{
	return ::operator new(s);
	/*
	//descartado hasta ahcer sistema eficiente
	//Lock lck(owner->mMemPoolCS);
	//std::scoped_lock<std::mutex> lck(owner->mMemPoolCS);
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
	throw std::bad_alloc();
	*/
	
}

void ::mel::tasking::_private::RunnableTask::operator delete( void* ptr ) noexcept
{
	::operator delete(ptr);	
/*	Runnable* ownerRunnable;
	RTMemBlock* block = (RTMemBlock*)((char*)ptr - offsetof( RTMemBlock, task));
	ownerRunnable = block->owner->owner;
	//Lock lck(ownerRunnable->mMemPoolCS);
	//std::scoped_lock<std::mutex> lck(ownerRunnable->mMemPoolCS);
	block->memState = RTMemBlock::EMemState::FREE;
	if (--block->owner->count == 0 )
	{
		//remove pool only if it's not the first pool
		if ( ownerRunnable->mRTZone.size() > 1 )
			ownerRunnable->_removePool( block->owner );
	}*/
	
}
void ::mel::tasking::_private::RunnableTask::operator delete(void* ptr, Runnable*) noexcept
{
	RTMemBlock* mBlock = (RTMemBlock*)((char*)ptr - offsetof(RTMemBlock, task));
	mBlock->memState = RTMemBlock::EMemState::FREE;
}

std::function<bool()> Runnable::killTrue;
std::function<bool()> Runnable::killFalse([]{return false;});
RTMemPool* Runnable::_addNewPool()
{
	RTMemPool* result;
	RTMemPool auxPool;
	//reserve memory and mark all blocks as free 
	auxPool.owner = this;
	auxPool.pool = (RTMemBlock*)malloc( sizeof(RTMemBlock)*mOpts.maxPoolSize );
	auxPool.count = 0;
	mRTZone.push_front(std::move(auxPool));
	//@todo guardar iterador
	auto poolIterator = mRTZone.getList().begin();
	//poolIterator->iterator = poolIterator;
	result = &(*poolIterator);
	for ( unsigned int i = 0; i < mOpts.maxPoolSize; ++i )
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

Runnable::Runnable(RunnableCreationOptions options ):
	mCurrentInfo(nullptr),
	//mState(State::INITIALIZED),	
	mTasks(options.schedulerOpts),
	mOpts(std::move(options)),
	mOwnerThread(0)  //assume 0 is invalid thread id!!
{
	//create one default pool
	gCurrrentRunnableCS.lock();
	
	
	if (!gCurrentRunnableKeyCreated)
	{
		TLS::createKey(gCurrentRunnableKey);		
		gCurrentRunnableKeyCreated = true;
	}
	
	gCurrrentRunnableCS.unlock();
	//_addNewPool(); quitado hasta tener todo claro, pero parece que incluso inicializar el pool cuesta mucho cuando es grande
	//Create default timer. Can be overriden by calling
	setTimer(std::make_shared<Timer>());
	mDefaultFactory = std::make_unique<ProcessFactory>();
}

Runnable::~Runnable() {
	mTasks.destroyAllProcesses();
	for( auto& i:mRTZone.getList())
	{
		free(i.pool);
		
	}
	
}

void Runnable::setTimer(std::shared_ptr<Timer> timer )
{
	mTasks.setTimer( timer );
}

void Runnable::postTask(std::shared_ptr<Process> process, unsigned int startTime)
{
//	assert( process && "is NULL");
	mTasks.insertProcess( process,startTime );
	onPostTask( process );
}

void Runnable::processTasks()
{
	if ( mOwnerThread == 0)  //first time?
		mOwnerThread = ::mel::core::getCurrentThreadId(); 
	
	if ( !mCurrentInfo )  //now initialize curranble info
	{
		RunnableInfo* ri = _getCurrentRunnableInfo();
		if (ri == nullptr)
		{
			ri = new RunnableInfo; //@todo ahora quedar� esta perdida de memoria
			TLS::setValue(gCurrentRunnableKey, ri);
		}
		mCurrentInfo = ri;
	}
	Runnable* oldR = mCurrentInfo->current;
	mCurrentInfo->current = this;
	mTasks.executeProcesses();
	mCurrentInfo->current = oldR;
}

