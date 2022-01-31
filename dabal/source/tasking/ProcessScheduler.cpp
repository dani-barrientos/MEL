
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

ProcessScheduler::ProcessScheduler():
	mTimer( nullptr )
	//mRequestedTaskCount( 0 )
	,mProcessCount( 0 ),
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
	if (mProcessCount == 0)
		return;
	TProcessList::iterator i;
	TProcessList::iterator end;
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

	//don't block if no new processes. size member is not atomic, but if a new prodess is being inserted in this moment, it will be inserted in next iteration
	if ( !mNewProcesses.empty())
	{
		mCS.enter();
		end = mNewProcesses.end();
		for( i = mNewProcesses.begin(); i != end; ++i )
		{
			mProcessList.push_back( *i );
			(*i).first->mLastUpdateTime = time; 
			(*i).first->setProcessScheduler( this );
		}
		mNewProcesses.clear();	
		mCS.leave();
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
				i = processes.erase( i );
			}else
				++i;
			mProcessInfo->current = nullptr;		
			/*	
			if ( state == Process::EProcessState::TRYING_TO_KILL )
			{
				//call kill again
				p->kill();
			}
			unsigned int lap = (unsigned int)(time-p->getLastUpdateTime());
			//const auto mask = Process::EProcessState::PREPARED | Process::EProcessState::PREPARED_TO_DIE | TRYING_TO_KILL; 

			if ( state == Process::EProcessState::PREPARED )
			{
				if ( lap >= i->second)
					p->update(time); 
			}else if ( state != Process::EProcessState::PREPARED_TO_DIE && lap > p->getPeriod())  //lo tenía mal al no hacer update en trying to kill
				p->update(time); 			

			//check if Process is trying to kill, then retry kill
			if ( p->getState() == Process::EProcessState::TRYING_TO_KILL )
			{
				p->kill();
			}else
			//not in else because maybe previous code gets a prepared to die Process
			if( p->getPreparedToDie())
			{
				//proceso muerto, lo anoto para quitarlo
				//removes this process from pending
				//mPendingIdTasksCS.enter(); para que es esto
				p->setDead();
				//mPendingIdTasks.erase( p->getId() );
				//mProcessCount--; proteger?? puedo usar un atomic pero es un poco ridiculo
				mProcessCount.fetch_sub(1,::std::memory_order_relaxed);

				//mPendingIdTasksCS.leave();
				//get next Process in its chain (if any)

				p->setProcessScheduler( NULL ); //nobody is scheduling the process
				mES.triggerCallbacks(p);
				i = processes.erase( i );
			}else
			{
				++i;
			}
			*/
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
	TProcessList::iterator	i;
	for(i = mProcessList.begin() ; i != mProcessList.end(); ++i)
	{
		(*i).first->kill();
	}
	mCS.enter();
	for( i = mNewProcesses.begin(); i != mNewProcesses.end(); ++i)
	{
		(*i).first->kill();
	}

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

	Lock lck(mCS);  
	//mProcessCount++;
	mNewProcesses.push_back( std::make_pair(process,startTime) );
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