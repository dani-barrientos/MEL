#include <core/Event_mthread.h>
using core::Event_mthread;

#include <tasking/ProcessScheduler.h>
using tasking::ProcessScheduler;

Event_mthread::Event_mthread(bool autoRelease, bool signaled): 
	mSignaled( signaled ),
	mAutoRelease( autoRelease )
{
}
Event_mthread::~Event_mthread()
{
	//TODO despertar los procesos?
}
void Event_mthread::set( bool sendToAll )
{
	core::Lock lck(mCS);
	mSignaled = true;
	_triggerActivation( sendToAll );
	if ( mAutoRelease )
	{
		reset();
	}
}

Event_mthread::EWaitCode Event_mthread::_wait( unsigned int msecs ) 
{
	EWaitCode result = EVENTMT_WAIT_OK;
	mCS.enter();
	if ( !mSignaled )
	{
		auto p = ProcessScheduler::getCurrentProcess();
		mWaitingProcesses.push_back( p ); //remember. not multithread-safe
        
		Process::ESwitchResult switchResult;
		if ( msecs == EVENTMT_WAIT_INFINITE )
			switchResult = ::core::Process::sleepAndDo([this]()
			{
				mCS.leave();
			} );
		else
			switchResult = ::core::Process::waitAndDo(msecs, [this]()
			{
				mCS.leave();
			});
		switch ( switchResult )
		{
		case Process::ESwitchResult::ESWITCH_KILL:
			result = EVENTMT_WAIT_KILL;
			break;
		case Process::ESwitchResult::ESWITCH_WAKEUP:
			result = EVENTMT_WAIT_OK; 
			break;
		default:
			result = EVENTMT_WAIT_TIMEOUT;
			break;
		}
		//remove process form list. It's safe to do it here because current process is already waiting (now is returning)
		//maybe other process do wait on this events
        mCS.enter();
		mWaitingProcesses.remove( p );
        mCS.leave();
	}else
		mCS.leave();
	/*if ( !mSignaled )
	{
		auto p = ProcessScheduler::getCurrentProcess();
        mCS.enter();
		mWaitingProcesses.push_back( p ); //remember. not multithread-safe
        mCS.leave();
        
		Process::ESwitchResult switchResult;
		if ( msecs == EVENTMT_WAIT_INFINITE )
			switchResult = ::core::Process::sleep( );
		else
			switchResult = ::core::Process::wait( msecs );
		switch ( switchResult )
		{
		case Process::ESwitchResult::ESWITCH_KILL:
			result = EVENTMT_WAIT_KILL;
			break;
		case Process::ESwitchResult::ESWITCH_WAKEUP:
			result = EVENTMT_WAIT_OK; 
			break;
		default:
			result = EVENTMT_WAIT_TIMEOUT;
			break;
		}
		//remove process form list. It's safe to do it here because current process is already waiting (now is returning)
		//maybe other process do wait on this events
        mCS.enter();
		mWaitingProcesses.remove( p );
        mCS.leave();
	}
	*/
	return result;
}


void Event_mthread::reset()
{
	mSignaled = false;
}
void Event_mthread::_triggerActivation( bool sendToAll )
{
    //core::Lock lck(mCS);
    
	if ( sendToAll )
	{
		for( TProcessList::iterator i = mWaitingProcesses.begin(); i != mWaitingProcesses.end(); ++i )
			(*i)->wakeUp();
		mWaitingProcesses.clear();
	}else
	{

		if ( !mWaitingProcesses.empty() )
		{
			mWaitingProcesses.front()->wakeUp();
			mWaitingProcesses.pop_front();
		}
	}
}