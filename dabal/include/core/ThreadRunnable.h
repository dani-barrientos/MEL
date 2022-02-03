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
	*

     ACTUALIZAR DOC
	* A user can create a GenericThread if he wants to execute a functor in a separate
	* thread. This functor must obey to signature: bool f( Thread* thread,bool kill)
	*	Where:
	*		- param[in] thread is the GenericThread container that use it
	*		- param[in] kill. There is a petition of killing this thread, so you must take into account and
	*			return true if you can terminate
	*		- bool return. if you want the thread to terminate, return true
	*
	* If terminate is called outside thread, the functor should consider it (kill argument) and return true if it can exit, so
	* thread will not finish until that functor returns true
	* The thread is automatically destroyed when finished if autoDestroy parameter == true (@todo Should it be in Thread class?)
	* @remarks The functor is executed using onCycleEnd because is more efficient than using a task
	* but it will not be killed if you do Runnable::getTaskScheduler().killProcesses(), you must to use Thread::finish (the usual way)
	*
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
		
		static std::shared_ptr<ThreadRunnable> create( bool autoRun = true, bool autoDestroy = true,unsigned int maxTasksSize = Runnable::DEFAULT_POOL_SIZE )
		{
            auto th = new ThreadRunnable(autoDestroy,maxTasksSize);
			//auto result = std::make_shared<ThreadRunnable>( autoDestroy,maxTasksSize );
            std::shared_ptr<ThreadRunnable> result(th);
			if (autoRun)
                result->run();
			return result;
		}
        ~ThreadRunnable();
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
		bool		mAutoDestroy;
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
		* @param autoDestroy Thread will be deleted when finished ->REVISAR, NO BIEN IMPLEMENTADO		
		*/
        ThreadRunnable( bool autoDestroy = true,unsigned int maxTasksSize = Runnable::DEFAULT_POOL_SIZE );
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
