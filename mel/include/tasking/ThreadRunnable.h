#pragma once
/*
 * SPDX-FileCopyrightText: 2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
#include <core/Thread.h>
using mel::core::Thread;
#include <tasking/Runnable.h>
using mel::tasking::Runnable;
//can alternate between custom "clasic" events to C++11 condition_variables which seems to get better performance at least on Windows platforms
#ifndef USE_CUSTOM_EVENT
#include <condition_variable>
#endif

namespace mel
{
    namespace tasking
    {
        /**
        * @class ThreadRunnable
        * @brief Thread with Runnable behaviour. 
        * @details
        * An instance of this class allow to receive task via Runnable available functions. The thread will be paused if no active tasks in its scheduler, that means:
        * there isn't any task or the tasks are in sleep state. One a task is posted or a task is awake, the thread continues processing
        * Destruction of a ThreadRunnable implies a join, waiting for the tasks to be completed. A kill signal is sent to all the tasks, but *not forced*, so is user's responsability
        * to manage tasks correctly
        */
        class MEL_API ThreadRunnable : public Runnable
        ///@cond HIDDEN_SYMBOLS
            ,public std::enable_shared_from_this<ThreadRunnable>
        ///@endcond
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
             * @param opts creation options
             * @return std::shared_ptr<ThreadRunnable> 
             */
            static std::shared_ptr<ThreadRunnable> create( bool autoRun = true,Runnable::RunnableCreationOptions opts = sDefaultOpts)
            {
                auto th = new ThreadRunnable(std::move(opts));
                th->start();
                std::shared_ptr<ThreadRunnable> result(th);
                if (!autoRun)  //really means auto pause if no autorun, always create running
                    result->suspend();
                return result;
            }
            ~ThreadRunnable();
            /**
             * @brief Start Thread
             * Internally calls Runnablle::processTask in an infinite loop, with some other check for performance and state control
             * virtual protected onRun is called before thread loop is set just in case children need to do custom job at that moment
             * 
             */
            void start();
            // void finish() override { terminate(0); }
            // bool finished()	override {return getState() == THREAD_FINISHED;	}
            /**
             * @brief Does a join on the underlying Thread
             * @details when join is done, the function \ref onJoined is called, as a way to notify children
             */
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
            void terminate();
            bool getTerminateRequest(){ return mEnd; }
            /**
            * return current executing ThreadRunnable. nullptr if any.
            * result can be NULL if current executing thread is not a Thread type (for example, main application thread
            * or thread created through API functions
            */
            static ThreadRunnable* getCurrentThreadRunnable();
        private:
            static Runnable::RunnableCreationOptions sDefaultOpts;
            std::unique_ptr<Thread> mThread;
        #ifdef USE_CUSTOM_EVENT
            Event	mWaitForTasks; 
        #else
            volatile bool mSignaled;
            std::condition_variable	mWaitForTasksCond;
            std::mutex mCondMut;
        #endif
            volatile bool mEnd; // request
            EThreadState mState;
            ::mel::core::Event mPauseEV;
        
            ::mel::core::ECallbackResult _processAwaken(std::shared_ptr<Process> p);
            void _execute();       
            /**
            * suspend inmediately, can be called only from same thread execution
            */
            ::mel::tasking::EGenericProcessResult _suspendInternal(uint64_t millis,Process* proc) noexcept;
            void _signalWakeup();
        protected:
            /**
            * @brief Create a thread with an empty loop, that continuosly processes posted tasks
            */
            ThreadRunnable( Runnable::RunnableCreationOptions opts = sDefaultOpts) ;
            /**
             * @brief Called by start() function
             * Children can override it to add custom behaviour
             */
            virtual void onStart(){};
            /**
             * @brief Called one time at the start of the thread loop, so, already in this thread context
             * Don't confuse with onStart,which is called when start() is called, so called on the *caller thread*^context
             */
            virtual void onThreadStart(){}
            /**
             * @brief Called when at the end of the thread loop
             * 
             */
            virtual void onThreadEnd(){}
            /**
             * @brief Called by \ref join() when done. 
             * @details Intended for specializaations to be notified
             */
            virtual void onJoined(){}
            void onCycleEnd();

            /**
            * overriden from Runnable
            */
            void onPostTask(std::shared_ptr<Process> process) override;
        };    
        
    }
}