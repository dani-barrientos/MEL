#include <tasking/Event_mthread.h>
using mel::tasking::Event_mthread;
using mel::tasking::EventBase;
using mel::tasking::EventMTThreadSafePolicy;
using mel::tasking::EventNoMTThreadSafePolicy;
#include <tasking/ProcessScheduler.h>
using mel::tasking::ProcessScheduler;

EventBase::EventBase(bool autoRelease, bool signaled):mSignaled( signaled ),
	mAutoRelease( autoRelease )
	{
		
	}

// EventBase::~EventBase()
// {
// 	//TODO despertar los procesos?
// }
void EventBase::_set( bool sendToAll )
{
	mSignaled = true;
	_triggerActivation( sendToAll );
	if ( mAutoRelease )
	{
		reset();
	}
}
void EventBase::reset()
{
	mSignaled = false;
}
void EventBase::_triggerActivation( bool sendToAll )
{   
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


EventMTThreadSafePolicy::EventMTThreadSafePolicy(bool autoRelease, bool signaled):EventBase(autoRelease,signaled)
{

}
tasking::EEventMTWaitCode EventMTThreadSafePolicy::_wait( unsigned int msecs ) 
{
	EEventMTWaitCode result = EEventMTWaitCode::EVENTMT_WAIT_OK;
	mCS.enter();
	if ( !EventBase::mSignaled )
	{
		auto p = ProcessScheduler::getCurrentProcess();
		EventBase::mWaitingProcesses.push_back( p ); //remember. not multithread-safe
		
		Process::ESwitchResult switchResult;
		if ( msecs == EVENTMT_WAIT_INFINITE )
			switchResult = ::mel::tasking::Process::sleepAndDo([this]()
			{
				mCS.leave();
			} );
		else
			switchResult = ::mel::tasking::Process::waitAndDo(msecs, [this]()
			{
				mCS.leave();
			});
		switch ( switchResult )
		{
		case Process::ESwitchResult::ESWITCH_KILL:
			result = EEventMTWaitCode::EVENTMT_WAIT_KILL;
			break;
		case Process::ESwitchResult::ESWITCH_WAKEUP:
			result = EEventMTWaitCode::EVENTMT_WAIT_OK; 
			break;
		default:
			result = EEventMTWaitCode::EVENTMT_WAIT_TIMEOUT;
			break;
		}
		//remove process form list. It's safe to do it here because current process is already waiting (now is returning)
		//maybe other process do wait on this events
		mCS.enter();
		mWaitingProcesses.remove( p );
		mCS.leave();
	}else
		mCS.leave();
	return result;
}

EventNoMTThreadSafePolicy::EventNoMTThreadSafePolicy(bool autoRelease, bool signaled):EventBase(autoRelease,signaled){}
tasking::EEventMTWaitCode EventNoMTThreadSafePolicy::_wait( unsigned int msecs ) 
{
	EEventMTWaitCode result = EEventMTWaitCode::EVENTMT_WAIT_OK;
	if ( !EventBase::mSignaled )
	{
		auto p = ProcessScheduler::getCurrentProcess();
		EventBase::mWaitingProcesses.push_back( p ); //remember. not multithread-safe
		
		Process::ESwitchResult switchResult;
		if ( msecs == EVENTMT_WAIT_INFINITE )
			switchResult = ::mel::tasking::Process::sleep();
		else
			switchResult = ::mel::tasking::Process::wait(msecs);
		switch ( switchResult )
		{
		case Process::ESwitchResult::ESWITCH_KILL:
			result = EEventMTWaitCode::EVENTMT_WAIT_KILL;
			break;
		case Process::ESwitchResult::ESWITCH_WAKEUP:
			result = EEventMTWaitCode::EVENTMT_WAIT_OK; 
			break;
		default:
			result = EEventMTWaitCode::EVENTMT_WAIT_TIMEOUT;
			break;
		}
		//remove process form list. It's safe to do it here because current process is already waiting (now is returning)
		//maybe other process do wait on this events
		mWaitingProcesses.remove( p );
	}
	return result;
}