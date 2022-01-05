#pragma once

#include <core/Thread.h>
using core::Thread_Impl;
using core::Thread;
//can alternate between custom "clasic" events to C++11 condition_variables which seems to get better performance at least on Windows platforms
#ifndef USE_CUSTOM_EVENT
#include <condition_variable>
#endif
namespace core
{
	/**
	* @class GenericThread
	* @brief Thread created from a functor
	*
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
	class DABAL_API GenericThread : public Thread_Impl< GenericThread >
	{
		friend class Thread_Impl<GenericThread>;
	public:

		/**
		* create new GenericThread.
		* @param[in] functor Functor to execute on each cycle. Signature is <bool,Thread*,bool>
		* @param[in] autoRun If true, thread start at construction. Otherwise you must call start manually
		* @param[in] autoDestroy If true (default) thread is deleted when finished
		* @param maxTasksSize the maximum number of tasks allowed on this thread
		*/
		template <class F> static 
		GenericThread* createGenericThread( F&& functor,bool autoRun = true, bool autoDestroy = true,unsigned int maxTasksSize = Runnable::DEFAULT_POOL_SIZE )
		{
            return new GenericThread( std::forward<F>(functor),autoRun, autoDestroy,maxTasksSize );
		}
		/**
		* @todo funcion para pruebas. no usar todavia
		*/
		static GenericThread* createEmptyThread( bool autoRun = true, bool autoDestroy = true,unsigned int maxTasksSize = Runnable::DEFAULT_POOL_SIZE )
		{
			return new GenericThread( autoRun, autoDestroy,maxTasksSize );
		}

		~GenericThread();
	protected:

		template <class F>
		GenericThread( F&& functor,bool autoRun, bool autoDestroy, unsigned int maxTaskSize );
        GenericThread( std::function<bool(Thread*,bool)>&& f,bool autoRun,bool autoDestroy,unsigned int maxTaskSize);
		GenericThread(const std::function<bool(Thread*, bool)>& f, bool autoRun, bool autoDestroy, unsigned int maxTaskSize);
		GenericThread( bool autoRun, bool autoDestroy, unsigned int maxTaskSize);
		/**
		* copy constructor. Only to hide to users
		*/
		GenericThread( GenericThread& t ):Thread_Impl<GenericThread>(""){};
		/**
		* default constructor. Only to hide to users
		*/
		GenericThread():Thread_Impl<GenericThread>(""){}
	private:
		::core::ECallbackResult _processAwaken(std::shared_ptr<Process> p);
		Callback< bool, Thread*,bool >*	mThreadFunction;
		bool						mAutoDestroy;
		bool						mTerminateAccepted;
	
	#ifdef USE_CUSTOM_EVENT
		Event	mWaitForTasks; 
	#else
		bool mSignaled;
		std::condition_variable	mWaitForTasksCond;
		std::mutex mCondMut;  //para pruebas con la condition varaible
	#endif
	protected:
		/**
		* overriden from Thread
		*/
		void onThreadEnd() override;
		void onCycleEnd();
		/**
		* overridden from Thread
		*/
		bool terminateRequest() override;

		/**
		* overriden from Runnable
		*/
		unsigned int onPostTask(std::shared_ptr<Process> process,ETaskPriority priority) override;
		//! overriden from Thread
		void terminate(unsigned int exitCode) override;
	};
	template <class F>
	GenericThread::GenericThread( F&& functor, bool autoRun, bool autoDestroy,unsigned int maxTasksSize ):
		Thread_Impl<GenericThread>( "",maxTasksSize ),
		mAutoDestroy( autoDestroy ),
		mTerminateAccepted( false )
	{
		getTasksScheduler().susbcribeWakeEvent(makeMemberEncapsulate(&GenericThread::_processAwaken, this));
        mThreadFunction = new Callback<bool,Thread*,bool>( ::std::forward<F>(functor),::core::use_functor );
		if ( autoRun )
		{
			start(); 
		}
	}

	
}
