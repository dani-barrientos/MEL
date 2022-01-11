
///////////////////////////////////////////////////////////
//  ProcessScheduler.cpp
//  Implementation of the Class ProcessScheduler
//  Created on:      29-mar-2005 10:00:12
///////////////////////////////////////////////////////////

#include <core/ProcessScheduler.h>
#include <algorithm>
#include <functional>

using core::ProcessScheduler;

#include <core/GenericProcess.h>
using core::GenericProcess;

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
mRequestedTaskCount( 0 )
,mTimer( NULL )
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
			
			//@todo el sleeped no debería ser un estado?? eso de un boleano no cuadra mucho
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
			//const auto mask = Process::EProcessState::PREPARED | Process::EProcessState::PREPARED_TO_DIE/* | TRYING_TO_KILL*/; ya no

			if ( state == Process::EProcessState::PREPARED )
			{
				if ( lap >= i->second)
					p->update(time); 
			}else if ( state != Process::EProcessState::PREPARED_TO_DIE && lap > p->getPeriod())
				p->update(time); 			
			mProcessInfo->current = nullptr;				
			
			//check if Process is trying to kill, then retry kill
			if ( p->getState() == Process::EProcessState::TRYING_TO_KILL )
			{
				p->kill();
			}
			//not in else because maybe previous code gets a prepared to die Process
			if( p->getPreparedToDie())
			{
				//proceso muerto, lo anoto para quitarlo
				//removes this process from pending
				mPendingIdTasksCS.enter();
				p->setDead();
				mPendingIdTasks.erase( p->getId() );
				mProcessCount--;
				mPendingIdTasksCS.leave();
				//get next Process in its chain (if any)
				/*if ( p->getNext() )
				{
					mNew.push_back( p->getNext() );
				}*/
				p->setProcessScheduler( NULL ); //nobody is scheduling the process
				EvictSubscriptor::second.triggerCallbacks(p);
				i = processes.erase( i );
			}else
			{
				++i;
			}
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
			addParam<::core::EGenericProcessResult,::core::EGenericProcessState, uint64_t,Process*,void>
			(
				addParam<::core::EGenericProcessResult,Process*, uint64_t,void>
				(
					addParam<::core::EGenericProcessResult, uint64_t,void>
					(
						returnAdaptor<void>
						(
							makeMemberEncapsulate( &ProcessScheduler::_killTasks, this )
							,::core::EGenericProcessResult::KILL
						)
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

unsigned int ProcessScheduler::insertProcess(std::shared_ptr<Process> process, unsigned int startTime)
{
	TProcessList::iterator	i;
	unsigned int newId;

	if (process == nullptr)
		return 0;
	mCS.enter();
	//mark process as pending for execution
	mPendingIdTasksCS.enter();
	//mPendingIdTasks[ newId ] = process;

	auto pos = mPendingIdTasks.find( process->getId() );
	if ( pos == mPendingIdTasks.end() )
	{
		newId = ++mRequestedTaskCount;

		process->setId( newId );
		mPendingIdTasks.insert({ newId, process });
		mProcessCount++;
		mNewProcesses.push_back( std::make_pair(process,startTime) );
	}else
	{
		//ya exist�a!
		newId = process->getId();
	}

	mPendingIdTasksCS.leave();
	mCS.leave();
	return newId;
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
	SleepSubscriptor::second.triggerCallbacks(p);
}
void ProcessScheduler::_triggerWakeEvents(std::shared_ptr<Process> p)
{
	WakeSubscriptor::second.triggerCallbacks(p);
}