
///////////////////////////////////////////////////////////
//  ProcessScheduler.cpp
//  Implementation of the Class ProcessScheduler
//  Created on:      29-mar-2005 10:00:12
///////////////////////////////////////////////////////////

#include <tasking/ProcessScheduler.h>
#include <algorithm>
#include <functional>

using mel::tasking::ProcessScheduler;

#include <tasking/GenericProcess.h>
using mel::tasking::GenericProcess;

#include <mpl/MemberEncapsulate.h>
using mel::mpl::makeMemberEncapsulate;
#include <mpl/ParamAdder.h>
using mel::mpl::addParam;
#include <mpl/ReturnAdaptor.h>
using mel::mpl::returnAdaptor;
#include <core/TLS.h>
using mel::core::TLS;
#include <cassert>
#include <core/Thread.h> //para pruebas, QUITAR!!!
#include <limits>
#undef max

static TLS::TLSKey	gTLSCurrentProcessKey;
static bool gTLSInited = false;
static CriticalSection gPSCS;

//initial memory for new posted tasks
ProcessScheduler::LockFreeTasksContainer::LockFreeTasksContainer(size_t chunkSize,size_t maxChunks):
	mChunkSize(chunkSize),mMaxSize(maxChunks*chunkSize),mInvalidate(false)
{
	if ( mMaxSize == 0 )
		mMaxSize = chunkSize;
	//Create one chunk. it will grow when needed
	mPool.emplace_back(chunkSize);
	mSize = chunkSize;
}
ProcessScheduler::LockFreeTasksContainer::PoolType::value_type& ProcessScheduler::LockFreeTasksContainer::operator[](size_t idx)
{	
	size_t nPool = idx/mChunkSize;
	size_t newIdx = idx%mChunkSize;
	
	return mPool[nPool][newIdx];
}
size_t ProcessScheduler::LockFreeTasksContainer::exchangeIdx(size_t v,std::memory_order order)
{
	return mCurrIdx.exchange(v,order);
}
void ProcessScheduler::LockFreeTasksContainer::add(std::shared_ptr<Process>& process,unsigned int startTime)
{	
	bool insert = true;
	size_t idx; 	
	//block caller until buffer reset is resolved
	while(mInvalidate){};  
	idx = mCurrIdx.fetch_add(1,::std::memory_order_release);  //no lo tengo claro todavía, estudiar
	if ( idx >= size())
	{
		Lock lck(mSC);
		//check size again
		size_t currSize = mSize.load(std::memory_order_relaxed);
		if ( idx >= currSize)
		{
			auto newSize = currSize + mChunkSize;
			mPool.emplace_back(mChunkSize);
			mSize.store(newSize,std::memory_order_release);
	//		spdlog::info("añado chunk. New size= {}",newSize);
		}
	}
	ElementType& element = operator[](idx);
	element.st = startTime;	
	element.p = std::move(process);
	element.valid.store(true,std::memory_order_release);
}
void ProcessScheduler::LockFreeTasksContainer::clear()
{
	mPool.clear();
	mPool.emplace_back(mChunkSize);
	mSize = mChunkSize;
}

// void ProcessScheduler::LockFreeTasksContainer::lock()
// {
// 	Lock lck(mSC);
// }



ProcessScheduler::ProcessScheduler(SchedulerOptions opts):
	mLastIdx(0),
	mTimer( nullptr ),
	mOpts(std::move(opts)),
	mProcessCount( 0 ),
	mInactiveProcessCount( 0 ),
	mKillingProcess( false ),
	mProcessInfo(nullptr)
{
	mIsLockFree = std::holds_alternative<LockFreeOptions>(mOpts);
	if ( mIsLockFree)
	{
		const LockFreeOptions& op = std::get<LockFreeOptions>(mOpts);
		mLockFreeTasks.reset(new LockFreeTasksContainer(op.chunkSize,op.maxChunks));
	}

	gPSCS.enter();
	if ( !gTLSInited )
	{
		TLS::createKey( gTLSCurrentProcessKey);
		gTLSInited = true;
	}
	gPSCS.leave();
}


ProcessScheduler::~ProcessScheduler(void)
{
	TProcessList::iterator	i;
	for(i = mProcessList.begin() ; i != mProcessList.end(); ++i)
	{
		(*i).first->setProcessScheduler( NULL );
	}
}
/**
* =============================================================================
*/

void ProcessScheduler::executeProcesses()
{
	#ifndef NDEBUG
	int stack;
	if ( _stack == nullptr)
		_stack = &stack;
	else
	{
		assert(_stack==&stack && "ProcessScheduler::executeProcesses. Invalid stack!!");
	}
	#endif
	/*if (mProcessCount.load(std::memory_order_relaxed) == 0)
		return;
	*/
	if (mProcessInfo == nullptr)
	{
		auto pi = _getCurrentProcessInfo();
		if (pi == nullptr)
		{
			pi = new ProcessInfo; //will lead to a leak
			TLS::setValue(gTLSCurrentProcessKey, pi);
		}
		mProcessInfo = pi;
	}
	auto previousProcess = mProcessInfo->current;
	assert(	mTimer != NULL );
	uint64_t time= mTimer->getMilliseconds();
	if (mIsLockFree)
	{
		bool invalidate = false;
		bool resetBuffer = false;	
		size_t currSize = mLockFreeTasks->size();
		if ( currSize > mLockFreeTasks->getMaxSize())
		{
			//need to reset buffer, too big
			//spdlog::info("Muy grande: {}",currSize);
			mLockFreeTasks->setInvalidate(true); //yo creo que en esto está lo que no furrula
			invalidate = true;
		}
		int count = 0;
		constexpr int maxCount = 10; //number of iterations to check for new task and give time for possible posting threads to finish their operation
		do
		{
			bool empty;
			size_t endIdx = mLockFreeTasks->getCurrIdx(std::memory_order_acquire); 
			/*
			intentar mejoras:
			- lo de las iteraciones para ver si cambia el indice y el sleep es un ful. como mucho un yield
			- es curiosi porque no se cumpló nunca que cambiase el indice
			- no me gusta lo del element->valid->especialmente no me gusta tener tantos atomic
			*/
		
			LockFreeTasksContainer::ElementType* element;
			if ( count > 0 && mLastIdx != endIdx)
			{
				//para depuracion
				text::info("index changed in iteration {}",count);
			}
			for(size_t idx = mLastIdx;idx < endIdx; ++idx)
			{		
				//maybe producer thread hasn't inserted the process yet
				empty = true;
				element = &(*mLockFreeTasks)[idx];
				do{
					if ( element->valid)
					//if ( element->p != nullptr && element->st != std::numeric_limits<unsigned int>::max())
					{
						element->p->mLastUpdateTime = time; 
						element->p->setProcessScheduler( this );
						//mProcessList.push_front( TProcessList::value_type(std::move(element->p),element->st) );
						mProcessList.emplace_front( std::move(element->p),element->st );
						element->st = std::numeric_limits<unsigned int>::max();
						element->valid.store(false,std::memory_order_release);				
						empty = false;
					}
					/*else
						spdlog::info("VACIO");*/
				}while(empty);
			}
			mLastIdx = endIdx;
			if ( invalidate )
			{
				//very ugly methos until having a best approach. it should occur very,very few times, so 
				//it shouldn't impact on performance
				invalidate = (++count < maxCount);
				core::Thread::yield();  
				resetBuffer = true;
			}
		}while(invalidate);
		if ( resetBuffer )
		{
			mLockFreeTasks->exchangeIdx(0,std::memory_order_acquire);
			mLastIdx = 0;
			mLockFreeTasks->clear();
			mLockFreeTasks->setInvalidate(false);
			//text::info("Buffer reseteado");
		}	
	}else
	{
		//don't block if no new processes. size member is not atomic, but if a new process is being inserted in this moment, it will be inserted in next iteration
		if ( !mBlockingTasks.empty())
		{
			mCS.enter();
			TNewProcesses::reverse_iterator i;
			TNewProcesses::reverse_iterator end;
			end = mBlockingTasks.rend();
		
			for( auto i = mBlockingTasks.rbegin(); i != end; ++i )
			{			
				(*i).first->mLastUpdateTime = time; 
				(*i).first->setProcessScheduler( this );
				mProcessList.push_front( std::move(*i) );
			}
			mBlockingTasks.clear();	
			
			mCS.leave();
		}
	}
	_executeProcesses( time,mProcessList);
	mProcessInfo->current = previousProcess;
}

void ProcessScheduler::_executeProcesses( uint64_t time,TProcessList& processes )
{
	std::shared_ptr<Process> p;

	if ( !processes.empty() )
	{
		TProcessList::iterator i = processes.begin();
		TProcessList::iterator end = processes.end();
		//TProcessList::iterator prev;
		TProcessList::iterator prev = processes.before_begin();
		while( i != end )
		{
			p = i->first;
			
			if ( p->getAsleep())
			{
				prev = i++;
				continue;
			}
			mProcessInfo->current = p;
			const auto state = p->getState();	
			if ( state == Process::EProcessState::TRYING_TO_KILL )
			{
				//call kill again
				p->kill();
			}
			unsigned int lap = (unsigned int)(time-p->getLastUpdateTime());	
			switch(state)
			{
				case Process::EProcessState::PREPARED:
					//firt iteration
					if ( lap >= i->second)
						p->update(time); 
					break;
				case Process::EProcessState::TRYING_TO_KILL:
					p->kill();
				case Process::EProcessState::INITIATED:					
				default:			
					if ( lap >= p->getPeriod())
						p->update(time); 
					break;
			}
			if ( p->getState()==Process::EProcessState::PREPARED_TO_DIE)  //new state after executing!!
			{
				p->setDead();
				mProcessCount.fetch_sub(1,::std::memory_order_relaxed);
				p->setProcessScheduler( NULL ); //nobody is scheduling the process
				mES.triggerCallbacks(p);
				i = processes.erase_after(prev);				
				//i = processes.erase( i );
			}else
			{
				prev = i++;
			}
			mProcessInfo->current = nullptr;			
		}

	}
}
/**
* ! pausa todos los procesos
*/
void ProcessScheduler::pauseProcesses()
{
	TProcessList::iterator	i;
	for(i = mProcessList.begin() ; i != mProcessList.end(); ++i)
	{
		(*i).first->pause();
	}
}

/**
* =============================================================================
*/
void ProcessScheduler::activateProcesses()
{
	TProcessList::iterator	i;
	for(i = mProcessList.begin() ; i != mProcessList.end(); ++i)
	{
		(*i).first->wakeUp();
	}
}

void ProcessScheduler::_killTasks()
{
	for(auto i = mProcessList.begin() ; i != mProcessList.end(); ++i)
	{
		(*i).first->kill();
	}
	mCS.enter();
	/*
	@todo resolver
	for( auto i = mNewProcesses.begin(); i != mNewProcesses.end(); ++i)
	{
		(*i).first->kill();
	}*/

	mCS.leave();
	mKillingProcess = false;
}
void ProcessScheduler::killProcesses( bool deferred )
{
	if ( deferred )
	{
		if ( mKillingProcess )
		{
			//there is another pending kill task
			return;
		}
		mKillingProcess = true;
		auto task = std::make_shared<GenericProcess>();
		task->setProcessCallback(
			addParam<::mel::tasking::EGenericProcessResult,Process*, uint64_t,void>
			(
				addParam<::mel::tasking::EGenericProcessResult, uint64_t,void>
				(
					returnAdaptor<void>
					(
						makeMemberEncapsulate( &ProcessScheduler::_killTasks, this )
						,mel::tasking::EGenericProcessResult::KILL
					)
				)
			)			
		);
		insertProcess( task);

	}
	else
	{
		_killTasks();
	}
}

void ProcessScheduler::destroyAllProcesses()
{
	for(auto i = mProcessList.begin() ; i != mProcessList.end(); ++i)
	{
		(*i).first->setProcessScheduler( NULL );
	}
	mProcessList.clear();
	mBlockingTasks.clear();
	mLockFreeTasks=nullptr;
}

void ProcessScheduler::insertProcess(std::shared_ptr<Process> process, unsigned int startTime)
{
	if (process == nullptr)
		return;
	mProcessCount.fetch_add(1,::std::memory_order_relaxed);
	if ( mIsLockFree)
	{
		mLockFreeTasks->add( process,startTime );
	}else
	{
		Lock lck(mCS);
		mBlockingTasks.push_back( std::make_pair(process,startTime) );
	}
}


void ProcessScheduler::insertProcessNoLock( std::shared_ptr<Process> process,unsigned int startTime )
{
	if (process == nullptr)
		return;
	mProcessCount.fetch_add(1,::std::memory_order_relaxed);
	if ( mIsLockFree)
	{
		mLockFreeTasks->add( process,startTime );
	}
	{
		mBlockingTasks.push_back( std::make_pair(process,startTime) );
	}
}

void ProcessScheduler::setTimer(std::shared_ptr<Timer> timer )
{
 	mTimer = timer;
}
ProcessScheduler::ProcessInfo* ProcessScheduler::_getCurrentProcessInfo()
{
	if (gTLSInited)
		return (ProcessInfo*)TLS::getValue(gTLSCurrentProcessKey);
	else
		return nullptr;
}
std::shared_ptr<Process> ProcessScheduler::getCurrentProcess()
{	
	auto ri = _getCurrentProcessInfo();
	if (ri) //not multithread-safe but it shouldn't be a problem
	{
		return ri->current;
	}
	else
	{
		return nullptr;
	}
}
void ProcessScheduler::processAwakened(std::shared_ptr<Process> p)
{
	mInactiveProcessCount.fetch_sub(1,::std::memory_order_relaxed);
	_triggerWakeEvents(p);
}
void ProcessScheduler::processAsleep(std::shared_ptr<Process>p)
{
	mInactiveProcessCount.fetch_add(1,::std::memory_order_relaxed);
	_triggerSleepEvents(p);
}
void ProcessScheduler::_triggerSleepEvents(std::shared_ptr<Process> p)
{
	mSS.triggerCallbacks(p);
}
void ProcessScheduler::_triggerWakeEvents(std::shared_ptr<Process> p)
{
	mWS.triggerCallbacks(p);
}