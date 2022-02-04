#pragma once

#include <core/Thread.h>
using core::Thread;
#include <tasking/Runnable.h>
using tasking::Runnable;
//can alternate between custom "clasic" events to C++11 condition_variables which seems to get better performance at least on Windows platforms
#ifndef USE_CUSTOM_EVENT
#include <condition_variable>
#endif
namespace core
{
	/**
	* @class ThreadRunnable
	* @brief Thread with Runnable behaviour. 
	* An instance of this class allow to receive task via Runnable available functions. The thread will be paused if no active tasks in its scheduler, that means:
    * there isn't any task or the tasks are in sleep state. One a task is posted or a task is awake, the thread continues processing
    * Destruction of a ThreadRunnable implies a join, waiting for the tasks to be completed. A kill signal is sent to all the tasks, but *not forced*, so is user's responsability
    * to manage tasks correctly
	*/
	class DABAL_API ThreadRunnable : public Runnable
	{
	public:
        enum EThreadState { 
            THREAD_INIT = binary<1>::value,
            THREAD_RUNNING  = binary<10>::value,
            THREAD_SUSPENDED = binary<100>::value,
            THREAD_FINISHING = binary<1000>::value, 
            THREAD_FINISHING_DONE = binary<10000>::value,
            THREAD_FINISHED = binary<100000>::value
        };	
		/**
		 * @brief Create new ThreadRunnable
		 * 
		 * @param autoRun if false, thread will be created suspende (but really running). The reason to no call this parameter "createSuspended" is because legacy issues
		 * @param maxTasksSize 
		 * @return std::shared_ptr<ThreadRunnable> 
		 */
		static std::shared_ptr<ThreadRunnable> create( bool autoRun = true,unsigned int maxTasksSize = Runnable::DEFAULT_POOL_SIZE )
		{
            auto th = new ThreadRunnable(maxTasksSize);
            th->start();
            std::shared_ptr<ThreadRunnable> result(th);
			if (!autoRun)  //really means auto pause if no autorun, always create running
                result->suspend();
			return result;
		}
        ~ThreadRunnable();
        /**
         * @brief Start Thread and con Runnable::run on its thread function
         * @todo esto está así para ir tirando, pero es poco consistente con el interfaz de Runnable
         * 
         */
        void start();
        void finish() override { terminate(0); }
        bool finished()	override {return getState() == THREAD_FINISHED;	}
		bool join(unsigned int millis=0xFFFFFFFF);
        bool setAffinity(uint64_t aff){return mThread->setAffinity(aff);}

        
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
        EThreadState getState() const{return mState;}
        void terminate(unsigned int exitCode);
        bool getTerminateRequest(){ return mEnd; }
        
	private:
        std::unique_ptr<Thread> mThread;
	#ifdef USE_CUSTOM_EVENT
		Event	mWaitForTasks; 
	#else
		bool mSignaled;
		std::condition_variable	mWaitForTasksCond;
		std::mutex mCondMut;  //para pruebas con la condition varaible
	#endif
        volatile bool mEnd; // request
        EThreadState mState;
        Event mPauseEV;
	
		::core::ECallbackResult _processAwaken(std::shared_ptr<Process> p);
        void _execute();       
        /**
        * suspend inmediately, can be called only from same thread execution
        */
		::tasking::EGenericProcessResult _suspendInternal(uint64_t millis,Process* proc);
	protected:
        /**
		* @brief Create a thread with an empty loop, that continuosly processes posted tasks @see Runnable::post
		*/
        ThreadRunnable( unsigned int maxTasksSize = Runnable::DEFAULT_POOL_SIZE );
        virtual void onThreadStart(){}
        virtual void onThreadEnd(){}
        virtual void onJoined(){}
		void onCycleEnd();

        /**
		* overriden from Runnable
		*/
        unsigned int onRun() override;
		/**
		* overriden from Runnable
		*/
		void onPostTask(std::shared_ptr<Process> process) override;
		
	};    
	
}
