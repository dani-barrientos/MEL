#pragma once

#include <core/Event.h>  
#include <core/ThreadDefs.h>


// #include <core/CallbackSubscriptor.h>
// using core::CallbackSubscriptor;
#include <memory>
#include <parallelism/Barrier.h>
#include <core/Future.h>
#if defined (DABAL_LINUX) || defined (DABAL_MACOSX) || defined(DABAL_ANDROID) || defined (DABAL_IOS)
#include <pthread.h>
#endif
#include <functional>

namespace core 
{
	DABAL_API unsigned int getNumProcessors();
	DABAL_API uint64_t getProcessAffinity();
	//!Set affinity for current thread.
	DABAL_API bool setAffinity(uint64_t);  
	using std::string;
	
	 /**
	 * @class Thread
	 * Platform-independent thread implementation.
	 * Provides generic methods for performing common thread-based tasks like
	 * handling priorities, sleeping, joining, and exposes additional methods for 
	 * queuing external tasks to be executed within the main thread loop (see Thread::post).<br>
	 * @note Threads are always created as _suspended_, and once starte are not suspended again by default
	 * when there are no any task to execute. This can be changed at will via setSuspendWhenNoTasks.
	 * @warning Some of these features may not be present on certain platforms, or may have
	 * specific requirements.
	 * 
	 * Please note that it's generally better to inherit from Thread_Impl insted of Thread directly, as
	 * it provides a few off-the-shelf features like task execution and few more things. Thread can of course
	 * be inherited, but will require additional programming to handle things safely.
	 * 
	 */
	class DABAL_API Thread/*: 
					public CallbackSubscriptor<::core::CSNoMultithreadPolicy,Thread*>*/
	{

#ifdef _WINDOWS
		friend DWORD WINAPI _threadProc(void* /*__in LPVOID*/);
#else
		friend void* _threadProc(void* param);
#endif

		DABAL_CORE_OBJECT_TYPEINFO_ROOT;
		public:
			enum YieldPolicy {
				YP_ANY_THREAD_ANY_PROCESSOR=0,
				YP_ANY_THREAD_SAME_PROCESSOR
			};
			
			/**
			 * Starts the main thread routine.
			 * Calling this function will force the main thread routine (Thread::run) to be spawned 
			 * and will return automatically just after doing so.<br>
			 * The thread will keep runing in background until Thread::run finishes.<br>
			 * Thread status can be queried meanwhile through Thread::isRunning, and the final
			 * result obtained via Thread::getResult, once it finishes.<br>
			 * Callers are encouraged to allways call Thread::join before attempting to query the
			 * result value or even exiting the main application.
			 */		
			template <class F> Thread(F&& f);
			//not a thread
			Thread();
			/**				 
			 * Creates a new thread with the given name.
			 * New threads are always created in a suspended state, meaning no actual thread
			 * process will be actually executed until Thread::start is invoked.
			 * @remarks Thread should be only deleted when is sure it's finished (see terminate and join)
			 */
			virtual ~Thread();

			/**
			 * Changes thread priority.
			 * May not be available in some platforms.
			 * @param tp the new priority to be set.				 
			 */
			void setPriority(ThreadPriority tp);
			/**
			 * Query thread's current priority.
			 * @return the priority of the thread
			 */
			inline ThreadPriority getPriority() const;

			
			
			
			/**
			 * Forces the caller to wait for thread completion.
			 * Calling this method will cause the calling thread to be wait until
			 * Thread:run finishes or the timeout expires.
			 * @param millis maximum milliseconds to wait for the thread.
			 * @return true if the thread finished. false if timeout 
			 */
			bool join(unsigned int millis=0xFFFFFFFF/*TODO mac: INFINITE*/);
			/**
			 * Get the thread execution result.
			 * @return the requested result, that will only be meaningful if the
			 * thread already finished. The meaning (if any) of the returned value is defined
			 * by the concrete subclass implementing the thread.
			 * @see join, isRunning
			 */
			unsigned int getResult() const;
			/**
			 * Get thread's running status
			 * @return true if the thread is still running. false otherwise
			 */
			inline bool isRunning() const;

			/**
			 * Forces the calling thread to sleep.
			 * Calling this method ensures the OS will schedule some CPU for any pending processes and
			 * threads that need attention.<br>
			 * NOTE: on some platforms, calling sleep(0) may cause the call to be completely ignored.
			 * @param millis the number of milliseconds to sleep for. The actual sleep time may
			 * depend on each platform, but you can expect a granularity not finer than 10ms, meaning
			 * sleep(1) will make the thread sleep for almost the same time as sleep(10).
			 */
			static void sleep(const unsigned int millis);
			/**
			 * Forces the calling thread to yield execution.
			 * This may have slight different effects depending on the platform, but theoretically
			 * the execution is yielded only to threads in the same process as the caller.<br>
			 * Calling this method may not force the OS to schedule any CPU time for any pending processes
			 * different that the caller.
			 */
			static void yield(YieldPolicy yp=YP_ANY_THREAD_SAME_PROCESSOR);

			
			/**
			* return handle for this thread
			*/
			inline ThreadId getThreadId() const;			
			/**
			* Get thread state
			* @return the thread's current state
			*/
			//inline EThreadState getState() const;
			/**
			* returns if a terminate request is done (mEnd == true)
			*/
			//inline bool getTerminateRequest() const;

			uint64_t getAffinity() const;
			/**
			* @todo no est� protegido frente a llamada con el hilo iniciandose
			*/
			bool setAffinity(uint64_t);
			/**
			* return minimun time(msecs) the system will wait with accuracy.
			* it depends on underlying OS and hardware, but traditionally it's about 10-15 msecs
			* @todo for the moment we will return a fixed contant depending on platform
			*/
			constexpr static unsigned getMinimunSleepTime()
			{
				//@todo por ahora pongo tiempo fijo "t�pico" para que se pueda usar y ya trataremos de que sea autom�tico o al menos m�s flexible
				constexpr unsigned MINIMUM_SLEEP = 10;
				return MINIMUM_SLEEP;				
			}
			/**
			* Forces thread termination
			* Use with extreme caution; this method exists but it should
			* rarely be used; it's always safer to invoke terminatRequest and
			* wait for the Thread to terminate "naturally", instead of forcing
			* it to quite.
			* @param exitCode the exit code for the terminated thread.
			*/
			void terminate(unsigned int exitCode=0);
		private:
			Thread(const char* name);
			enum class EJoinResult{JOINED_NONE,JOINED_OK,JOINED_ERROR} mJoinResult;
#ifdef DABAL_WINDOWS
			HANDLE mHandle = 0;
			DWORD mID;
#elif defined (DABAL_LINUX) || defined (DABAL_MACOSX) || defined(DABAL_ANDROID) || defined (DABAL_IOS)
		ThreadId mHandle = 0;
        #if !defined (DABAL_MACOSX) && !defined(DABAL_IOS)
		pid_t mThHandle = 0; //depending on posix functions used, (the miriad of them) use diferent handles types, etc
		#endif
			int	mPriorityMin;
			int mPriorityMax;
			uint64_t mAffinity = 0; //affinity to set on start. if 0, is ignored
#endif
#if defined (DABAL_MACOSX) || defined(DABAL_IOS)
			void* mARP; //The autorelease pool as an opaque-type
#endif
			unsigned int mExitCode;
			ThreadPriority mPriority;

			std::function<void()> mFunction;
			void _initialize();
			void _start();
	};
	template <class F> Thread::Thread(F&& f):mFunction(std::forward<F>(f))
	{
		_initialize();
		_start();
	}
	// bool Thread::getTerminateRequest() const
	// {
	// 	return mEnd;
	// }


	ThreadId Thread::getThreadId() const
	{
#ifdef _WINDOWS
		return mID;
#else
		return mHandle;
#endif
	}
	
	
	ThreadPriority Thread::getPriority() const {
		return mPriority;
	}
	/*bool Thread::getSuspendenWhenNoTasks() const
	{
		return mSuspenOnNoTasks;
	}*/




	/**
	* waiting for a Future from a Thread
	* @see tasking::waitForFutureMThread
	*/
	template<class T,class ErrorType = ::core::ErrorInfo> typename core::Future<T,ErrorType>::ValueType& waitForFutureThread( core::Future<T,ErrorType>& f,unsigned int msecs = ::core::Event::EVENT_WAIT_INFINITE)
    {
        using ::core::Event;
        struct _Receiver
        {		
            _Receiver():mEvent(false,false){}
			using futT = core::Future<T,ErrorType>;
            typename core::Future<T,ErrorType>::ValueType& wait( futT& f,unsigned int msecs)
            {
                Event::EWaitCode eventresult;				
            // spdlog::debug("Waiting for event in Thread {}",threadid);
				int evId = f.subscribeCallback(
					std::function<::core::ECallbackResult( const typename futT::ValueType&)>([this](const typename futT::ValueType& ) 
					{
						mEvent.set();
					//   spdlog::debug("Event was set for Thread {}",threadid);
						return ::core::ECallbackResult::UNSUBSCRIBE; 
					}));
				
                eventresult = mEvent.wait(msecs); 
				f.unsubscribeCallback(evId);
            //  spdlog::debug("Wait was done in Thread {}",threadid);
                switch( eventresult )
                {               
                case ::core::Event::EVENT_WAIT_TIMEOUT:
					f.setError(ErrorType(::core::EWaitError::FUTURE_WAIT_TIMEOUT,"Time out exceeded"));
                    break;
                case ::core::Event::EVENT_WAIT_ERROR:
					f.setError(ErrorType(::core::EWaitError::FUTURE_UNKNOWN_ERROR,"Unknown error"));
					break;
                }			        
				return f.getValue();
            }
            private:
            	::core::Event mEvent;
        };
        auto receiver = std::make_unique<_Receiver>();
        return receiver->wait(f,msecs);	
    }	
	DABAL_API ::core::Event::EWaitCode waitForBarrierThread(const ::parallelism::Barrier& b,unsigned int msecs = Event::EVENT_WAIT_INFINITE);
}
