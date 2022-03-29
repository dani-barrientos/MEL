#pragma once

#include <DabalLibType.h>
#if defined (DABAL_LINUX) || defined (DABAL_MACOSX) || defined(DABAL_ANDROID) || defined (DABAL_IOS)
#include <pthread.h>
#endif
namespace core {

	/**
	 * Platform-independent Event implementation.
	 * Can be used in multi-threaded scenarios to create simple semaphore-like
	 * locks on which clients can wait and notify in order to synchronize execution flows.
	 */
	class DABAL_API Event {
		public:
			/**
			 * @brief Wait result codes
			 */
			enum EWaitCode 
			{
				EVENT_WAIT_OK, //!< wait was ok
				EVENT_WAIT_TIMEOUT, //!< time out while waiting
				EVENT_WAIT_ERROR //!< unknown error
			}; 
#if defined (DABAL_LINUX) || defined (DABAL_MACOSX) || defined(DABAL_ANDROID) || defined (DABAL_IOS)
			static const int EVENT_WAIT_INFINITE = -1;
#elif defined(_WIN32)		
			static const int EVENT_WAIT_INFINITE = INFINITE;

#endif

			/**
			 * Creates a new event.
			 * @param autoRelease flag indicating if the new vent should be auto-reset whenever a wait
			 * operation terminates (the default value is true)
			 * @param signaled flag indicating the initial status of the event. If set to true, the event will
			 * be created as "signaled" meaning the next wait operation will return immediately.
			 */
			Event(bool autoRelease=true, bool signaled=false);
			~Event();

			/**
			 * Sets the event.
			 * If there were more than one thread locked on an Event::wait call, only one of them will be unlocked.
			 * If there were no threads waiting, then the next one calling Event:wait won't be locked.
			 * @see wait
			 */
			void set();

			/**
			 * Wait for event.
			 * If the event was not yet set, the calling thread will be locked until the event is set.
			 * @param[in] msecs milliseconds to wait for.
			 * @return return code.
			 * @see set
			 */
			EWaitCode wait( unsigned int msecs = EVENT_WAIT_INFINITE) const; 
			/**
			 * Resets the event, so any further calls to Event::wait will force a lock on the calling thread.
			 * @see wait, set
			 */
			void reset();

		private:
			Event( const Event& ev2 ){}
#if defined(DABAL_LINUX) || defined (DABAL_MACOSX) || defined(DABAL_ANDROID) || defined(DABAL_IOS)
			mutable bool mSignaled;
			bool mAutoRelease;
			mutable pthread_mutex_t _mutex;
			mutable pthread_cond_t _cond;
#elif defined(_WIN32)		
		HANDLE	mEvent;

#endif
};

//end namespace
}