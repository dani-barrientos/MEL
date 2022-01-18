#pragma once

#include <tasking/Runnable.h>
#include <core/Event.h>
using tasking::Runnable;

#include <mpl/MemberEncapsulate.h>
using mpl::makeMemberEncapsulate;

#include <core/ThreadDefs.h>

#include <mpl/ReturnAdaptor.h>
using mpl::returnAdaptor;

#include <mpl/Linker.h>
using mpl::link1st;

#include <core/CallbackSubscriptor.h>
using core::CallbackSubscriptor;
#include <memory>
#include <parallelism/Barrier.h>
namespace core {
	DABAL_API unsigned int getNumProcessors();
	DABAL_API uint64_t getProcessAffinity();
	//!Set affinity for current thread.
	DABAL_API bool setAffinity(uint64_t);  
	using std::string;
	/**
	 * @class Thread_Impl
	 */

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
	 * You can subscribe callback to thread end. It will be thrown just before onThreadEnd is called
	 */
	class DABAL_API Thread : public Runnable, public CallbackSubscriptor<::core::NoMultithreadPolicy,Thread*>
	{

#ifdef _WINDOWS
		friend DWORD WINAPI _threadProc(void* /*__in LPVOID*/);
#elif defined (DABAL_POSIX)
		friend void* _threadProc(void* param);
#endif

		DABAL_CORE_OBJECT_TYPEINFO;
		public:
			enum YieldPolicy {
				YP_ANY_THREAD_ANY_PROCESSOR=0,
				YP_ANY_THREAD_SAME_PROCESSOR
			};
			
			/**				 
			 * Creates a new thread with the given name.
			 * New threads are always created in a suspended state, meaning no actual thread
			 * process will be actually executed until Thread::start is invoked.
			 * @remarks Thread should be only deleted when is sure it's finished (see terminate and join)
			 */
			~Thread();

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
			 * Starts the main thread routine.
			 * Calling this function will force the main thread routine (Thread::run) to be spawned 
			 * and will return automatically just after doing so.<br>
			 * The thread will keep runing in background until Thread::run finishes.<br>
			 * Thread status can be queried meanwhile through Thread::isRunning, and the final
			 * result obtained via Thread::getResult, once it finishes.<br>
			 * Callers are encouraged to allways call Thread::join before attempting to query the
			 * result value or even exiting the main application.
			 */
			void start();
			/**
			 * Makes the main thread routine to be suspended.
			 * If thread is no in THREAD_RUNNING, it hasn't effect
			 * @return the number of pending "suspend" operations performed so far, or 0 if the
			 * thread could not be suspended.
			 *
			 * @warning. keep an eye on it because between state change to THREAD_PAUSED and real pause ther is a gap. 
			 * because is's done through a post
			 *	It could be fixed with SignalObjectAndWait in Windows but I don't know how to do it in POSIX
			 */
			unsigned int suspend();
			/**
			 * Restarts a previously suspended thread.
			 * @return the number of pending "suspend" operations before resume was called, or 0xFFFFFFFF
			 * if the operation was not successful. If the value returned is greater than 1, the thread is
			 * still suspended.
			 * NOTE: Calling this method has no effect on platforms others than Windows.
			 */
			unsigned int resume();
			/**
			* overridden from Runnable for compatibility
			* It calls terminate 
			* @todo revisar no est� nada claro
			*/
			void finish() override { terminate(); }
			/**
			* overridden from Runnable. For compatibility. 
			*/
			bool finished()	override {return mState == THREAD_FINISHED;	}
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
			* return current executing thread. NULL if any.
			* result can be NULL if current executing thread is not a Thread type (for example, main application thread
			* or thread created through API functions
			*/
			static Thread* getCurrentThread();
			/**
			* return handle for this thread
			*/
			inline ThreadId getThreadId() const;			
			/**
			* Get thread state
			* @return the thread's current state
			*/
			inline EThreadState getState() const;
			/**
			* returns if a terminate request is done (mEnd == true)
			*/
			inline bool getTerminateRequest() const;

			uint64_t getAffinity() const;
			/**
			* @todo no est� protegido frente a llamada con el hilo iniciandose
			*/
			bool setAffinity(uint64_t);

			inline void setCrashReportingEnabled(bool cr);
			inline bool isCrashReportingEnabled() const;

			static void setDefaultCrashReportingEnabled(bool cr);
			static bool isDefaultCrashReportingEnabled();
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
		protected:
			Thread(const char* name,unsigned int maxTaskSize = Runnable::DEFAULT_POOL_SIZE);
			
#ifdef _WINDOWS
			HANDLE mHandle;
			DWORD mID;
			
#elif defined (DABAL_POSIX)
		ThreadId mHandle = 0;
        #if !defined (DABAL_MACOSX) && !defined(DABAL_IOS)
		pid_t mThHandle = 0; //depending on posix functions used, (the miriad of them) use diferent handles types, etc
		#endif
			bool mJoined;
			int	mPriorityMin;
			int mPriorityMax;
			uint64_t mAffinity = 0; //affinity to set on start. if 0, is ignored
#endif
#if defined (DABAL_MACOSX) || defined(DABAL_IOS)
			void* mARP; //The autorelease pool as an opaque-type
#endif
			Event mPauseEV;
			string mName;
			unsigned int mResult;
			EThreadState mState;
			volatile bool mEnd; //end request
			bool mPausedWhenNoTasks; //true if was paused whene there aren't pending tasks. Que poco me gusta...
			unsigned int mExitCode;
			ThreadPriority mPriority;

			unsigned int runInternal();

			/**
			* suspend inmediately, can be called only from same thread execution
			*/
			::tasking::EGenericProcessResult suspendInternal(uint64_t millis,Process* proc,::tasking::EGenericProcessState);
			/**
			* called when thread begins to run
			*/
			virtual void onThreadStart(){};
			/**
			* called when thread finish to run. It's called just after last instruction, so you can
			* delete thread if you want.
			*/
			virtual void onThreadEnd(){}; 
			/**
			 * @brief Called when join is done
			 * 
			 */
			virtual void onJoined(){};

			/**
			* terminate request. It will be called in execution loop when
			* a terminate is called. By default returns true, meaning that thread
			* inmediately goes in THREAD_FINISHING. This doesn't mean that thread will terminate,
			* because all task need to be finished
			*/
			virtual bool terminateRequest(){ return mEnd; }

			/**
			* Forces thread termination
			* Use with extreme caution; this method exists but it should
			* rarely be used; it's always safer to invoke terminatRequest and
			* wait for the Thread to terminate "naturally", instead of forcing
			* it to quite.
			* @param exitCode the exit code for the terminated thread.
			*/
			virtual void terminate(unsigned int exitCode=0);

		private:
			static bool DEFAULT_CR_ENABLED; //Global (default) crash reporting flag
			bool mCREnabled; //Per-instance crash reporting flag

			inline void threadEnd()
			{
				triggerCallbacks(this);
				onThreadEnd();
			}						
	};
	
	bool Thread::getTerminateRequest() const
	{
		return mEnd;
	}

	void Thread::setCrashReportingEnabled(bool cr) {
		mCREnabled = cr;
	}
	bool Thread::isCrashReportingEnabled() const {
		return mCREnabled;
	}

	ThreadId Thread::getThreadId() const
	{
#ifdef _WINDOWS
		return mID;
#else
		return mHandle;
#endif
	}
	bool Thread::isRunning() const 
	{
		return mState == THREAD_RUNNING;
	}
	EThreadState Thread::getState() const
	{
		return mState;
	}
	ThreadPriority Thread::getPriority() const {
		return mPriority;
	}
	/*bool Thread::getSuspendenWhenNoTasks() const
	{
		return mSuspenOnNoTasks;
	}*/

	template <class T> class Thread_Impl : public Thread
	{
	public:
		Thread_Impl(const char* name,unsigned int maxTaskSize = Runnable::DEFAULT_POOL_SIZE);
	protected:
		/**
		* overridden from Runnable
		* It makes an infinite loop calling processTask.
		* If there aren't any task to process or they are paused, then Thread is paused
		* and resumed if a task is added or resumed
		* at cycle begin onCycleBegin is called
		* at cycle end onCycleEnd is called
		* at begin it calls onThreadStart
		* @remarks this function is intended to be non-overridden but because it is virtual at Runanble
		* you can override it, but at your own risk, because many things loose their meaning, as for example, getState
		*/
		unsigned int onRun();

		/**
		* default actions por onCyclexxx
		* these functions are intented to be orriden by children (using compile-time polymorphism)
		*/
		void onCycleBegin(){};
		void onCycleEnd(){};
		void oneIteration()
		{
			((T*)this)->onCycleBegin();
			processTasks();
			//@todo dormir/levantar thread en funcion de las tareas
			((T*)this)->T::onCycleEnd();
		}

	};
	template <class T>
	Thread_Impl<T>::Thread_Impl(const char* name,unsigned int maxTaskSize) : 
		Thread( name,maxTaskSize )
		{
			
		}



	template <class T> unsigned int Thread_Impl<T>::onRun()	
	{
		onThreadStart();
		while ( mState != THREAD_FINISHING_DONE )
		{
			switch( mState )
			{
			case THREAD_RUNNING:
				if ( terminateRequest() )
				{
					//end order was received
					mState = THREAD_FINISHING;
					getTasksScheduler().killProcesses( false ); 
				}
				break;
			case THREAD_FINISHING:
				if ( getTasksScheduler().getProcessCount() == 0 ) //forma cutre de hacerlo, vale por ahora
				{
					mState = THREAD_FINISHING_DONE;
				}
				break;
            default:
                ;
			}
			oneIteration(); 
		}
		return 1; // != 0 no error
	}


	/**
	* waiting for a Future from a Thread
	* @see tasking::waitForFutureMThread
	*/
	template<class T> ::core::FutureData_Base::EWaitResult waitForFutureThread( const core::Future<T>& f,unsigned int msecs = ::core::Event::EVENT_WAIT_INFINITE)
    {
        using ::core::Event;
        using ::core::FutureData_Base;
        using ::core::FutureData;
        struct _Receiver
        {		
            _Receiver():mEvent(false,false){}
            FutureData_Base::EWaitResult wait(const core::Future<T>& f,unsigned int msecs)
            {
                FutureData_Base::EWaitResult result;            
                Event::EWaitCode eventresult;				
            // spdlog::debug("Waiting for event in Thread {}",threadid);
				int evId = f.subscribeCallback(
					std::function<::core::ECallbackResult( const FutureData<T>&)>([this](const FutureData<T>& ) 
					{
						mEvent.set();
					//   spdlog::debug("Event was set for Thread {}",threadid);
						return ::core::ECallbackResult::UNSUBSCRIBE; 
					})
				);
                eventresult = mEvent.wait(msecs); 
				f.unsubscribeCallback(evId);
            //  spdlog::debug("Wait was done in Thread {}",threadid);
                switch( eventresult )
                {
                case ::core::Event::EVENT_WAIT_OK:
                    //event was triggered because a kill signal
                    result = ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_OK;
                    break;
                case ::core::Event::EVENT_WAIT_TIMEOUT:
                    result = ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_TIMEOUT;
                    break;
                case ::core::Event::EVENT_WAIT_ERROR:
                    result = ::core::FutureData_Base::EWaitResult::FUTURE_UNKNOWN_ERROR;
					break;
                }			
                return result;	
        
            }
            private:
            	::core::Event mEvent;
        };
        auto receiver = std::make_unique<_Receiver>();
        return receiver->wait(f,msecs);	
    }	
	::core::Event::EWaitCode waitForBarrierThread(const ::parallelism::Barrier& b,unsigned int msecs = Event::EVENT_WAIT_INFINITE);
}
