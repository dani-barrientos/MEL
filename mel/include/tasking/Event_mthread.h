#pragma once
/*
 * SPDX-FileCopyrightText: 2005,2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
#include <MelLibType.h>
#include <list>
#include <tasking/Process.h>
#include <core/Callback.h>
#include <tasking/ProcessScheduler.h>

namespace mel
{
	namespace tasking
	{
		using ::mel::tasking::Process;
		using std::list;
		using mel::core::Callback;
		enum class EEventMTWaitCode {
					EVENTMT_WAIT_OK,
					EVENTMT_WAIT_TIMEOUT,
					EVENTMT_WAIT_KILL};
		static const int EVENTMT_WAIT_INFINITE = -1;
		class MEL_API EventBase
		{
			public:
				EventBase(bool autoRelease=true, bool signaled=false);
				
				void reset();
			private:

				typedef list< std::shared_ptr<Process> > TProcessList;
				void _triggerActivation( bool sendToAll );
			protected:
				bool mSignaled;
				bool mAutoRelease;
				TProcessList mWaitingProcesses;
				void _set( bool sendToAll );

		};
		
		/**
		 * @brief Policy for multithread safe event
		 */
		class MEL_API EventMTThreadSafePolicy : public EventBase
		{
			public:
				EventMTThreadSafePolicy(bool autoRelease, bool signaled);
				void set(bool sendToAll)
				{
					core::Lock lck(mCS);
					EventBase::_set(sendToAll);
				}
			protected:
				EEventMTWaitCode _wait( unsigned int msecs ); 			
				template <class F>
				EEventMTWaitCode _waitAndDo( F postSleep,unsigned int msecs ) 
				{
					EEventMTWaitCode result = EEventMTWaitCode::EVENTMT_WAIT_OK;
					mCS.enter();
					if ( !mSignaled )
					{
						auto p = ::mel::tasking::ProcessScheduler::getCurrentProcess();
						mWaitingProcesses.push_back( p ); 
						
						Process::ESwitchResult switchResult;
						if ( msecs == EVENTMT_WAIT_INFINITE )
							switchResult = ::mel::tasking::Process::sleepAndDo([this,postSleep]()
							{
								mCS.leave();
								postSleep();
							} );
						else
							switchResult = ::mel::tasking::Process::waitAndDo(msecs, [this,postSleep]()
							{
								mCS.leave();
								postSleep();
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
					{
						mCS.leave();
						postSleep();
					}
					return result;
				}
			private:
				core::CriticalSection mCS;
		};
			/**
		 * @brief Policy for non-multithread safe event
		 */
		class MEL_API EventNoMTThreadSafePolicy : public EventBase
		{
			public:
				EventNoMTThreadSafePolicy(bool autoRelease, bool signaled);
				inline void set(bool sendToAll)
				{
					EventBase::_set(sendToAll);
				}
			protected:
				EEventMTWaitCode _wait( unsigned int msecs );			
				template <class F>
				EEventMTWaitCode _waitAndDo( F postSleep,unsigned int msecs ) 
				{
					EEventMTWaitCode result = EEventMTWaitCode::EVENTMT_WAIT_OK;
					if ( !mSignaled )
					{
						auto p = ::mel::tasking::ProcessScheduler::getCurrentProcess();
						mWaitingProcesses.push_back( p ); 
						
						Process::ESwitchResult switchResult;
						if ( msecs == EVENTMT_WAIT_INFINITE )
							switchResult = ::mel::tasking::Process::sleepAndDo([postSleep]()
							{
								postSleep();
							} );
						else
							switchResult = ::mel::tasking::Process::waitAndDo(msecs, [this,postSleep]()
							{
								postSleep();
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
						mWaitingProcesses.remove( p );
					}else
					{
						postSleep();
					}
					return result;
				}
				
		};
		/**
		* @brief class similar to Event Class (which is for thread synchronization) but for Process (with Microthread behaviour)	
		*/
		template <class MultithreadPolicy = EventMTThreadSafePolicy> class Event_mthread : public MultithreadPolicy
		{
		public:
			
			/**
			* Creates a new event.
			* @param autoRelease flag indicating if the new vent should be auto-reset whenever a wait
			* operation terminates (the default value is true)
			* @param signaled flag indicating the initial status of the event. If set to true, the event will
			* be created as "signaled" meaning the next wait operation will return immediately.
			*/
			Event_mthread(bool autoRelease=true, bool signaled=false):MultithreadPolicy(autoRelease,signaled){}
			//~Event_mthread();

			/**
			* activate event
			* @param[in] sendToAll If true then all attached Processed are awake else, only one in FIFO way
			*/
			void set( bool sendToAll = true ){MultithreadPolicy::set(sendToAll);}

			/**
			* wait for event to be triggered
			* @param[in] msecs to wait for
			* @return EEventMTWaitCode
			*/
			inline EEventMTWaitCode wait( unsigned int msecs = EVENTMT_WAIT_INFINITE)
			{
				return MultithreadPolicy::_wait( msecs );
			}

			/**
			* sleep Process, executing callback on sleep,until wakeup		
			* @return bool with same meaning as wait
			*/
			template <class F>
			EEventMTWaitCode waitAndDo( F postSleep,unsigned int msecs = EVENTMT_WAIT_INFINITE )
			{
				return MultithreadPolicy::_waitAndDo( postSleep,msecs );
			}

		private:

			
		};
	}
}