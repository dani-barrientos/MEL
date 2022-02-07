#include <core/ThreadRunnable.h>
using core::ThreadRunnable;
#include <mpl/MemberEncapsulate.h>


ThreadRunnable::ThreadRunnable( unsigned int maxTasksSize):Runnable(maxTasksSize),mState(THREAD_INIT),mEnd(false),
	mPauseEV(true,false),mThread(std::make_unique<Thread>())
{
}
void ThreadRunnable::_execute()	
{
	mState = THREAD_RUNNING;
    onThreadStart();
    while ( mState != THREAD_FINISHING_DONE )
    {
        switch( mState )
        {
        case THREAD_RUNNING:
            if ( getTerminateRequest() )
            {
                //end order was received
                mState = THREAD_FINISHING;
                getScheduler().killProcesses( false ); 
            }
            
            break;
        case THREAD_FINISHING:
            if ( getScheduler().getProcessCount() == 0 )
            {
                mState = THREAD_FINISHING_DONE;
            }
            break;
        default:;
        }
        processTasks();
        onCycleEnd();
    }
    onThreadEnd();
    mState = THREAD_FINISHED;
    //@todo  gestiono el audodestroye? no creo
    //return 1; // != 0 no error
}

void ThreadRunnable::start()
{
	onStart();
	mThread = std::make_unique<Thread>(mpl::makeMemberEncapsulate(&ThreadRunnable::_execute,this));
}

bool ThreadRunnable::join(unsigned int millis)
{
    bool result = mThread->join(millis);
    onJoined();
    return result;
}
::tasking::EGenericProcessResult ThreadRunnable::_suspendInternal(uint64_t millis,Process* proc) {
	mPauseEV.wait();
	return ::tasking::EGenericProcessResult::KILL;
}
unsigned int ThreadRunnable::suspend()
{
	if ( mState == THREAD_RUNNING )
	{
		post(makeMemberEncapsulate(&ThreadRunnable::_suspendInternal,this)); //post as the first task for next iteration
		mState=THREAD_SUSPENDED; //aunque todav�a no se haya hecho el wait,
								 //por la forma en que funcionan los eventos da igual, porque si se hace
								 //un resume justo antes de procesarse la tarea "suspendInternal", implica que el set
								 //hace que el siguiente wait no espere, lo cual es lo correcto
	}
	return 1;
}

unsigned int ThreadRunnable::resume() {    
	if ( mState == THREAD_SUSPENDED )
	{
		mState = THREAD_RUNNING;
#ifdef _WINDOWS
		//DWORD sc=ResumeThread(mHandle);
		mPauseEV.set();
	}
	//return mHandle?sc:0;
	return 1;
#endif
#if defined (DABAL_POSIX)
		mPauseEV.set();
	}
	return 1;

#endif
}
ThreadRunnable::~ThreadRunnable()
{
    finish();
}
/*
void ThreadRunnable::onThreadEnd()
{
	Thread::onThreadEnd();	
	//@todo estoy no es correcto, hay que hacerlo bien
	if ( mAutoDestroy )
	 	delete this;
}

*/
void ThreadRunnable::onPostTask(std::shared_ptr<Process> process)
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

::core::ECallbackResult ThreadRunnable::_processAwaken(std::shared_ptr<Process> p)
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
void ThreadRunnable::onCycleEnd()
{
	unsigned int count = 0;	
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
void ThreadRunnable::terminate(unsigned int exitCode)
{
	if ( mState == THREAD_SUSPENDED )
	{
		resume();
	}else if ( mState == THREAD_INIT )
	{
	/*	//TODO vigilar, esto no est� bien. Puede ocurrir que en este momento est� arrancado el hilo
		#ifdef _WINDOWS
		//in Windows we need to ResumeThread if it wasn't started. Other way it won't be removed from memory( I don't know why..)
		ResumeThread( mHandle );
		#endif*/
	}
    mEnd = true;
    #ifdef USE_CUSTOM_EVENT
	mWaitForTasks.set();
#else
	{
		std::lock_guard<std::mutex> lk(mCondMut);
		mSignaled = true;
	}
	mWaitForTasksCond.notify_one();
#endif
	//??mExitCode = exitCode;
}