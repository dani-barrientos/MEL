#pragma once
#include <DabalLibType.h>
#include <list>
#include <tasking/Process.h>
#include <core/Callback.h>
#include <tasking/ProcessScheduler.h>

namespace core
{
	using ::tasking::Process;
	using std::list;
	using core::Callback;
	/**
	* class similar to Event Class (which is for thread synchronization) but for Process (with Microthread behaviour)
	* Not multithread safe
	* Note: Process using it must be stored in SmartPtr because internally holds a SmartPtr list
	* 
	*/
	class DABAL_API Event_mthread
	{
	public:
		enum EWaitCode {EVENTMT_WAIT_OK,EVENTMT_WAIT_TIMEOUT,EVENTMT_WAIT_ERROR,EVENTMT_WAIT_KILL};
		static const int EVENTMT_WAIT_INFINITE = -1;
		/**
		* Creates a new event.
		* @param autoRelease flag indicating if the new vent should be auto-reset whenever a wait
		* operation terminates (the default value is true)
		* @param signaled flag indicating the initial status of the event. If set to true, the event will
		* be created as "signaled" meaning the next wait operation will return immediately.
		*
		* TODO hacer subscripci�n a eventos
		*/
		Event_mthread(bool autoRelease=true, bool signaled=false);
		~Event_mthread();

		/**
		* activate event
		* @param[in] sendToAll If true then all attached Processed are awake else, only one in FIFO way
		*/
		void set( bool sendToAll = true );

		/**
		* wait for event to be triggered
		* @param[in] msecs to wait for
		* @return EWaitCode
		*/
		inline EWaitCode wait( unsigned int msecs = EVENTMT_WAIT_INFINITE)
		{
			return _wait( msecs );
		}

		/**
		* sleep Process, executing callback on sleep,until wakeup
		* internally use a SmartPtrList
		* @return bool with same meaning as wait
		*/
		template <class F>
		EWaitCode waitAndDo( F postSleep,unsigned int msecs = EVENTMT_WAIT_INFINITE )
		{
			return _waitAndDo( postSleep,msecs );
		}

		void reset();
	private:
		bool mSignaled;
		bool mAutoRelease;

		typedef list< std::shared_ptr<Process> > TProcessList;
		TProcessList mWaitingProcesses;
        core::CriticalSection mCS;//!@todo: revisar esto, que es un arreglo chapucero para solucionar un "hueco" evidente en los accesos a mWaitingProcesses


		void _triggerActivation( bool sendToAll );
		EWaitCode _wait( unsigned int msecs );

		template <class F>
		EWaitCode _waitAndDo( F postSleep,unsigned int msecs ) 
		{
			EWaitCode result = EVENTMT_WAIT_OK;
			if ( !mSignaled )
			{
				auto p = ::tasking::ProcessScheduler::getCurrentProcess();
                mCS.enter();
				mWaitingProcesses.push_back( p );
                mCS.leave();
				
                Process::ESwitchResult switchResult;
				if ( msecs == EVENTMT_WAIT_INFINITE )
					switchResult = ::tasking::Process::sleepAndDo( postSleep );
				else
					switchResult = ::tasking::Process::waitAndDo( msecs,postSleep );
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
			else
			{
				//El evento ya se se�al�, pero ejecutamos igualmente el "post sleep"
				postSleep();
			}
			return result;
		}
	};
}