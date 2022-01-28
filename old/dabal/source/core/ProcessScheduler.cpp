
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

	for( i = mInitialProcesses.begin(); i != mInitialProcesses.end(); ++i)
	{
		(*i)->setProcessScheduler( NULL );

	}
	for(i = mProcessList.begin() ; i != mProcessList.end(); ++i)
	{
		(*i)->setProcessScheduler( NULL );
	}
	for( i = mFinalProcesses.begin(); i != mFinalProcesses.end(); ++i)
	{
		(*i)->setProcessScheduler( NULL );

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
	//Process* previousProcess = (Process*)TLS::getValue( gTLSCurrentProcessKey ); //TODO por alguna raz�n en VTS lo hac�a en cada executeProcesses..
	
	assert(	mTimer != NULL );
	uint64_t time= mTimer->getMilliseconds();
	//inserto procesos pendientes
	//mNew.clear();
	mCS.enter();
	end = mNewProcesses.end();
	for( i = mNewProcesses.begin(); i != end; ++i )
	{
		mProcessList.push_back( *i );
		(*i)->mLastTime = time;
		(*i)->setProcessScheduler( this );
	}
	mNewProcesses.clear();
	end = mNewInitialProcesses.end();
	for( i = mNewInitialProcesses.begin(); i != end; ++i )
	{
		mInitialProcesses.push_back( *i );
		(*i)->setProcessScheduler( this );
		(*i)->mLastTime = time;
	}
	mNewInitialProcesses.clear();
	end = mNewFinalProcesses.end();
	for( i = mNewFinalProcesses.begin(); i != end; ++i )
	{
		mFinalProcesses.push_back( *i );
		(*i)->setProcessScheduler( this );
		(*i)->mLastTime = time;
	}
	mNewFinalProcesses.clear();
	mCS.leave();

	executeProcesses( time,mInitialProcesses);
	executeProcesses( time,mProcessList);
	executeProcesses( time,mFinalProcesses);

	/*for (auto p : mNew)
	{
		insertProcess(p, NORMAL);
	}	*/
	//TLS::setValue( gTLSCurrentProcessKey,previousProcess);
	mProcessInfo->current = previousProcess;
}

void ProcessScheduler::executeProcesses( uint64_t time,TProcessList& processes )
{
	std::shared_ptr<Process> p;

	if ( !processes.empty() )
	{
		TProcessList::iterator i = processes.begin();
		TProcessList::iterator end = processes.end();
		while( i != end )
		{
			p = *i;
			if( p->getActive() ) //si no est� activo paso de �l
			{			
				mProcessInfo->current = p;
				p->onUpdate( time );
				mProcessInfo->current = nullptr;				
			}
			//check if Process is trying to kill, then retry kill
			if ( p->getState() == Process::TRYING_TO_KILL )
			{
				p->kill();
			}
			//not in else becasue maybe previous code gets a prepared to die Process
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
				EvictSubscriptor::triggerCallbacks(p);
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

	for( i = mInitialProcesses.begin(); i != mInitialProcesses.end(); ++i)
	{
		(*i)->pause();
	}
	for(i = mProcessList.begin() ; i != mProcessList.end(); ++i)
	{
		(*i)->pause();
	}
	for( i = mFinalProcesses.begin(); i != mFinalProcesses.end(); ++i)
	{
		(*i)->pause();
	}

}



/**
* =============================================================================
*/
void ProcessScheduler::activateProcesses()
{
	TProcessList::iterator	i;

	for( i = mInitialProcesses.begin(); i != mInitialProcesses.end(); ++i)
	{
		(*i)->activate();
	}
	for(i = mProcessList.begin() ; i != mProcessList.end(); ++i)
	{
		(*i)->activate();
	}
	for( i = mFinalProcesses.begin(); i != mFinalProcesses.end(); ++i)
	{
		(*i)->activate();
	}

}


/**
* =============================================================================
*/
void ProcessScheduler::killTask()
{
	TProcessList::iterator	i;

	for( i = mInitialProcesses.begin(); i != mInitialProcesses.end(); ++i)
	{
		(*i)->kill();
	}
	for(i = mProcessList.begin() ; i != mProcessList.end(); ++i)
	{
		(*i)->kill();
	}
	for( i = mFinalProcesses.begin(); i != mFinalProcesses.end(); ++i)
	{
		(*i)->kill();
	}
	mCS.enter();
	for( i = mNewInitialProcesses.begin(); i != mNewInitialProcesses.end(); ++i)
	{
		(*i)->kill();
	}
	for( i = mNewProcesses.begin(); i != mNewProcesses.end(); ++i)
	{
		(*i)->kill();
	}
	for( i = mNewFinalProcesses.begin(); i != mNewFinalProcesses.end(); ++i)
	{
		(*i)->kill();
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
				addParam<bool,Process*, uint64_t,void>
				(
					addParam<bool, uint64_t,void>
					(
						returnAdaptor<void>
						(
							makeMemberEncapsulate( &ProcessScheduler::killTask, this )
							,true
						)
					)
				)			
		);
		insertProcess( task,HIGH );

	}
	else
	{
		killTask();
	}
}

void ProcessScheduler::destroyAllProcesses()
{
	list< std::shared_ptr<Process> >::iterator i;
	for( i = mInitialProcesses.begin(); i != mInitialProcesses.end(); ++i)
	{
		(*i)->setProcessScheduler( NULL );

	}
	for(i = mProcessList.begin() ; i != mProcessList.end(); ++i)
	{
		(*i)->setProcessScheduler( NULL );
	}
	for( i = mFinalProcesses.begin(); i != mFinalProcesses.end(); ++i)
	{
		(*i)->setProcessScheduler( NULL );

	}
	mInitialProcesses.clear();
	mProcessList.clear();
	mFinalProcesses.clear();
	mNewInitialProcesses.clear();
	mNewProcesses.clear();
	mNewFinalProcesses.clear();
}

unsigned int ProcessScheduler::insertProcess(std::shared_ptr<Process> process,EProcessPriority priority )
{
	TProcessList::iterator	i;
	TProcessList* usedList;

	switch( priority )
	{
	case HIGH:
		usedList = &mNewInitialProcesses;
		break;
	case NORMAL:
		usedList = &mNewProcesses;
		break;
	case LOW:
		usedList = &mNewFinalProcesses;
		break;
	}
	unsigned int newId;

	if (process == 0)
		return 0;
	mCS.enter();
	//mark process as pending for execution
	mPendingIdTasksCS.enter();
	//mPendingIdTasks[ newId ] = process;

	auto pos = mPendingIdTasks.find( process->getId() );
	if ( pos == mPendingIdTasks.end() )
	{
		//no exist�a el proceso
		newId = ++mRequestedTaskCount;

		process->setId( newId );
		mPendingIdTasks.insert({ newId, process });
		mProcessCount++;
		usedList->push_back( process );
	}else
	{
		//ya exist�a!
		newId = process->getId();
	}

	mPendingIdTasksCS.leave();
	mCS.leave();
	return newId;
}



void ProcessScheduler::ini()
{
	//@todo mTimer = Timer::getSingletonPtr();
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
	SleepSubscriptor::triggerCallbacks(p);
}
void ProcessScheduler::_triggerWakeEvents(std::shared_ptr<Process> p)
{
	WakeSubscriptor::triggerCallbacks(p);
}