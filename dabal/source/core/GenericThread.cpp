#include <core/GenericThread.h>
using core::GenericThread;

GenericThread::GenericThread( std::function<bool(Thread*,bool)>&& f,bool autoRun,bool autoDestroy,unsigned int maxTaskSize):
    Thread_Impl<GenericThread>("",maxTaskSize),
	mAutoDestroy(autoDestroy),
	mTerminateAccepted(false),
	mThreadFunction(0)
#ifdef USE_CUSTOM_EVENT
	,mWaitForTasks(true,false)
#else
	,mSignaled(false)
#endif
	
	
{
    //assert(false && "Still in development!!!");        
	getScheduler().susbcribeWakeEvent(makeMemberEncapsulate(&GenericThread::_processAwaken, this));	
    mThreadFunction = new Callback<bool,Thread*,bool>( std::move(f),::core::use_function );
    if ( autoRun ) {
        start();
    }
}
GenericThread::GenericThread(const std::function<bool(Thread*, bool)>& f, bool autoRun, bool autoDestroy, unsigned int maxTaskSize):
	Thread_Impl<GenericThread>("", maxTaskSize),
	mAutoDestroy(autoDestroy),
	mTerminateAccepted(false),
	mThreadFunction(0)
#ifdef USE_CUSTOM_EVENT
	, mWaitForTasks(true, false)
#else
	,mSignaled(false)
#endif
{
	getScheduler().susbcribeWakeEvent(makeMemberEncapsulate(&GenericThread::_processAwaken, this));
	mThreadFunction = new Callback<bool, Thread*, bool>(f, ::core::use_function);
	if (autoRun) {
		start();
	}
}

GenericThread::GenericThread( bool autoRun, bool autoDestroy, unsigned int maxTaskSize):
	Thread_Impl<GenericThread>( "",maxTaskSize ),
	mAutoDestroy( autoDestroy ),
	mTerminateAccepted( false ),
	mThreadFunction( 0 )
#ifdef USE_CUSTOM_EVENT
	, mWaitForTasks(true, false)
#else
	,mSignaled(false)
#endif

{
	getScheduler().susbcribeWakeEvent(makeMemberEncapsulate(&GenericThread::_processAwaken, this));
	/*auto f = std::function<bool(Process*)>([](Process*) {
		return false;
	});
	getTasksScheduler().susbcribeSleepEvent(std::function<bool(Process*)>([](Process*) {
		return false;
	})
	);
	getTasksScheduler().susbcribeSleepEvent(f);*/
	if ( autoRun )
	{
		start(); 
	}
}
GenericThread::~GenericThread()
{
	terminateRequest();
	delete mThreadFunction;
}


void GenericThread::onThreadEnd()
{
	Thread::onThreadEnd();	
	//@todo estoy no es correcto, hay que hacerlo bien
	if ( mAutoDestroy )
	 	delete this;
}

bool GenericThread::terminateRequest()
{
	if ( mThreadFunction )
		return mTerminateAccepted; //only exit if thread function want it
	else
		return Thread_Impl< GenericThread >::terminateRequest();
}

void GenericThread::onPostTask(std::shared_ptr<Process> process)
{		
#ifdef USE_CUSTOM_EVENT
	mWaitForTasks.set();
#else
	{
		std::lock_guard<std::mutex> lk(mCondMut);
		mSignaled = true;
	}
	mWaitForTasksCond.notify_one();
	
#endif	


}
void GenericThread::terminate(unsigned int exitCode)
{
	Thread_Impl< GenericThread >::terminate( exitCode );
#ifdef USE_CUSTOM_EVENT
	mWaitForTasks.set();
#else
	{
		std::lock_guard<std::mutex> lk(mCondMut);
		mSignaled = true;
	}
	mWaitForTasksCond.notify_one();
#endif
}
::core::ECallbackResult GenericThread::_processAwaken(std::shared_ptr<Process> p)
{	
#ifdef USE_CUSTOM_EVENT
	mWaitForTasks.set();
#else
	{
		std::lock_guard<std::mutex> lk(mCondMut);
		mSignaled = true;
	}
	mWaitForTasksCond.notify_one();
#endif

	//::spdlog::debug("GenericThread::_processWaken");
	return ECallbackResult::NO_UNSUBSCRIBE;
}
void GenericThread::onCycleEnd()
{
	unsigned int count = 0;
	if (mThreadFunction)
	{
		if (!mTerminateAccepted)
		{
			mTerminateAccepted = (*mThreadFunction)(this, mEnd);
			//realmente no veo necesidad de "restaurar" el mEnd, ya que el terminateRequest() es quien manda
			//adem�s de que hay problemas de concurrencia
			//mEnd = mTerminateAccepted; //only exit if thread function want it
		}
	}
	else
	{
		
		count = getActiveTaskCount();
		if (!mEnd && count == 0)
		{
#ifdef USE_CUSTOM_EVENT
			mWaitForTasks.wait();
#else
			std::unique_lock<std::mutex> lk(mCondMut);
			mWaitForTasksCond.wait(lk, [this]
			{
				return mSignaled;
			}
			);
			mSignaled = false;	
#endif
		}
		
	}	
}