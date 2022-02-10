
///////////////////////////////////////////////////////////
//  ProcessScheduler.cpp
//  Implementation of the Class ProcessScheduler
//  Created on:      29-mar-2005 10:00:12
///////////////////////////////////////////////////////////

#include <tasking/ProcessScheduler.h>
#include <algorithm>
#include <functional>

using tasking::ProcessScheduler;

#include <tasking/GenericProcess.h>
using tasking::GenericProcess;

#include <mpl/MemberEncapsulate.h>
using mpl::makeMemberEncapsulate;
#include <mpl/ParamAdder.h>
using mpl::addParam;
#include <mpl/ReturnAdaptor.h>
using mpl::returnAdaptor;
#include <core/TLS.h>
using core::TLS;
#include <cassert>
#include <core/atomics.h>

static TLS::TLSKey	gTLSCurrentProcessKey;
static bool gTLSInited = false;
static CriticalSection gPSCS;
#ifdef PROCESSSCHEDULER_USE_LOCK_FREE	
//initial memory for new posted tasks
ProcessScheduler::NewTasksContainer::NewTasksContainer(size_t chunkSize,size_t maxSize):
	mChunkSize(chunkSize),mMaxSize(maxSize)
{
	if ( maxSize > chunkSize )
		mMaxSize = chunkSize;
	//Create one chunk. it will grow when needed
	mPool.emplace_back(chunkSize);
	//mPool.resize(chunkSize);
	mSize = chunkSize;
}
ProcessScheduler::NewTasksContainer::PoolType::value_type& ProcessScheduler::NewTasksContainer::operator[](size_t idx)
{	
	size_t nPool = idx/mChunkSize;
	size_t newIdx = idx%mChunkSize;
	
	return mPool[nPool][newIdx];
}
size_t ProcessScheduler::NewTasksContainer::exchangeIdx(size_t v)
{
	return mCurrIdx.exchange(v);
}
void ProcessScheduler::NewTasksContainer::add(std::shared_ptr<Process>& process,unsigned int startTime)
{	
	size_t idx; 
	idx = mCurrIdx.fetch_add(1,::std::memory_order_relaxed);  //no lo tengo claro todavía, estudiar
	if ( idx >= size())
	{
		Lock lck(mSC);
		size_t currSize = mSize.load(std::memory_order_relaxed);
		//check size again
		if ( idx >= currSize)
		{
			mPool.emplace_back(mChunkSize);
			auto newSize = currSize + mChunkSize;
			/*if ( newSize > mMaxSize)
			{
				poner a cero el indice y, ¿qué hago con el resto?

			}else*/
				mSize.store(newSize,std::memory_order_release);
		}
	}	
	ElementType& element = operator[](idx);
	element.st = startTime;	
	element.p = std::move(process);
	element.valid.store(true,std::memory_order_release);

}
void ProcessScheduler::NewTasksContainer::clear()
{
	mPool.clear();
	mPool.emplace_back(mChunkSize);
	mSize = 1;
}

void ProcessScheduler::NewTasksContainer::lock()
{
	Lock lck(mSC);
}
void ProcessScheduler::ProcessScheduler::resetPool()
{
	mNewProcesses.exchangeIdx(0);
	mLastIdx = 0;
	mNewProcesses.clear();
}
#endif
ProcessScheduler::ProcessScheduler(size_t initialPoolSize,size_t maxNewTasks):
	mLastIdx(0),
	mTimer( nullptr ),
#ifdef PROCESSSCHEDULER_USE_LOCK_FREE	
	mNewProcesses(initialPoolSize,maxNewTasks),
#endif
	mProcessCount( 0 ),
	mInactiveProcessCount( 0 ),
	mKillingProcess( false ),
	mProcessInfo(nullptr)
{
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
	if (mProcessCount.load(std::memory_order_relaxed) == 0)
		return;
	
	if (mProcessInfo == nullptr)
	{
		auto pi = _getCurrentProcessInfo();
		if (pi == nullptr)
		{
			pi = new ProcessInfo; //
			TLS::setValue(gTLSCurrentProcessKey, pi);
		}
		mProcessInfo = pi;
	}
	auto previousProcess = mProcessInfo->current;
	assert(	mTimer != NULL );
	uint64_t time= mTimer->getMilliseconds();
	//inserto procesos pendientes

	//don't block if no new processes. size member is not atomic, but if a new process is being inserted in this moment, it will be inserted in next iteration
	
	//pruebas con metodo viejo
#ifdef PROCESSSCHEDULER_USE_LOCK_FREE
	
	/*size_t endIdx = std::min(mNewProcesses.getCurrIdx(),mNewProcesses.size());
	size_t count = endIdx; esto no es correcto si mienrtas espero por los remaining
	se sobre pasas
	do
	{
		for(size_t idx = 0;idx < endIdx; ++idx)
		{
			auto& element = mNewProcesses[idx];
			//maybe producer thread hasn't inserted the process yet
			//@todo cuidado que es un pair, podría estar uinsertado el proceso y no el tiempo
			if ( element.first)
			{
				element.first->mLastUpdateTime = time; 
				element.first->setProcessScheduler( this );
				mProcessList.push_front( std::move(element) );
				--count;
			}
		}
	}while(count>0); //wait until all pendng elements are ready
	auto oldIdx = mNewProcesses.exchangeIdx(0);
	oldIdx = std::min(oldIdx,mNewProcesses.size());
	//process remaining processes until now
	count = oldIdx-endIdx;
	do
	{
		for(size_t idx = endIdx;idx < oldIdx; ++idx)
		{
			auto& element = mNewProcesses[idx];
			//maybe producer thread hasn't inserted the process yet
			//@todo cuidado que es un pair, podría estar uinsertado el proceso y no el tiempo
			if ( element.first)
			{
				element.first->mLastUpdateTime = time; 
				element.first->setProcessScheduler( this );
				mProcessList.push_front( std::move(element) );
				--count;
			}else
				spdlog::info("VACIO 2");
		}
	}while(count>0);
	*/

	//size_t endIdx = std::min(mNewProcesses.getCurrIdx(),mNewProcesses.size());	
	size_t endIdx = mNewProcesses.getCurrIdx();
	if ( endIdx > mNewProcesses.size() )
	{
		mNewProcesses.lock();
	}
	bool empty;
	NewTasksContainer::ElementType* element;
	for(size_t idx = mLastIdx;idx < endIdx; ++idx)
	{		
		//maybe producer thread hasn't inserted the process yet
		empty = true;
		//auto& element = mNewProcesses[idx];
		element = &mNewProcesses[idx];
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
	/*
	mNewProcesses.block(); //avoid producer threads to overcome this idx temporaryly
	auto oldIdx = mNewProcesses.exchangeIdx(0);
	if ( oldIdx > mNewProcesses.size() )
	{
		mNewProcesses.lock();
	}
	//process remaining processes until now

	for(size_t idx = endIdx;idx < oldIdx; ++idx)
	{
		//maybe producer thread hasn't inserted the process yet
		//@todo cuidado que es un pair, podría estar uinsertado el proceso y no el tiempo
		empty = true;
		auto& element = mNewProcesses[idx];
		do
		{
		//	if ( element.valid)
			if ( element.p != nullptr)
			{
				element.p->mLastUpdateTime = time; 
				element.p->setProcessScheduler( this );
				mProcessList.push_front( TProcessList::value_type(std::move(element.p),element.st) );
				//element.valid.store(false,std::memory_order_release);				
				empty = false;
			 }
			 //else
			// 	spdlog::info("VACIO");		
		}while(empty);
	}
	mNewProcesses.unblock();//remove barrier
	*/


#else
	if ( !mNewProcesses.empty())
	{
		mCS.enter();
		TNewProcesses::reverse_iterator i;
		TNewProcesses::reverse_iterator end;
		end = mNewProcesses.rend();
	
		for( auto i = mNewProcesses.rbegin(); i != end; ++i )
		{			
			(*i).first->mLastUpdateTime = time; 
			(*i).first->setProcessScheduler( this );
			mProcessList.push_front( std::move(*i) );
		}
		mNewProcesses.clear();	
		
		mCS.leave();
	}
#endif
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
		TProcessList::iterator prev = processes.before_begin();
		Process::EProcessState state;
		while( i != end )
		{
			p = i->first;
			
			if ( p->getAsleep())
			{
				++i;
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
			addParam<::tasking::EGenericProcessResult,Process*, uint64_t,void>
			(
				addParam<::tasking::EGenericProcessResult, uint64_t,void>
				(
					returnAdaptor<void>
					(
						makeMemberEncapsulate( &ProcessScheduler::_killTasks, this )
						,::tasking::EGenericProcessResult::KILL
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
	mNewProcesses.clear();
}

void ProcessScheduler::insertProcess(std::shared_ptr<Process> process, unsigned int startTime)
{
	if (process == nullptr)
		return;
	mProcessCount.fetch_add(1,::std::memory_order_relaxed);
#ifdef PROCESSSCHEDULER_USE_LOCK_FREE
	mNewProcesses.add( process,startTime );
#else
	Lock lck(mCS);
	mNewProcesses.push_back( std::make_pair(process,startTime) );
#endif
}


void ProcessScheduler::insertProcessNoLock( std::shared_ptr<Process> process,unsigned int startTime )
{
	if (process == nullptr)
		return;
	mProcessCount.fetch_add(1,::std::memory_order_relaxed);

#ifdef PROCESSSCHEDULER_USE_LOCK_FREE
	mNewProcesses.add( process,startTime );
#else
	mNewProcesses.push_back( std::make_pair(process,startTime) );
#endif
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
	::core::atomicDecrement( &mInactiveProcessCount );
	_triggerWakeEvents(p);
}
void ProcessScheduler::processAsleep(std::shared_ptr<Process>p)
{
	::core::atomicIncrement(&mInactiveProcessCount);
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