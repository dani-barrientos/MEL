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
	getTasksScheduler().susbcribeWakeEvent(makeMemberEncapsulate(&GenericThread::_processAwaken, this));	
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
	getTasksScheduler().susbcribeWakeEvent(makeMemberEncapsulate(&GenericThread::_processAwaken, this));
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
	getTasksScheduler().susbcribeWakeEvent(makeMemberEncapsulate(&GenericThread::_processAwaken, this));
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
	delete mThreadFunction;
}


void GenericThread::onThreadEnd()
{
	Thread::onThreadEnd();
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

unsigned int GenericThread::onPostTask(std::shared_ptr<Process> process,ETaskPriority priority)
{
	unsigned int result;
	/*//PENSAR QUE PASARIA SI HUBIESE MAS DE UN suspend ENCADENADO
	mSuspendCS.enter();
	//tambi�n se recibe un onPostTask en el suspend, pero tal y como esta hecho (ver Thread) el mState se fija despu�s del post
	if ( getState() == THREAD_SUSPENDED )
	{
		cout << "RESUMO\n";
		resume();
	}
	result = Thread_Impl< GenericThread >::onPostTask( process, priority );;
	mSuspendCS.leave();
	*/
//@remarks. If post is done before thread start running, the event is set before wait, so when task finish, event will be set and thread don't stop on event and will do an extrga cycle
	result = Thread_Impl< GenericThread >::onPostTask( process, priority );
	
#ifdef USE_CUSTOM_EVENT
	mWaitForTasks.set();
#else
	{
		std::lock_guard<std::mutex> lk(mCondMut);
		mSignaled = true;
	}
	mWaitForTasksCond.notify_one();
#endif	
	return result;
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
	return ECallbackResult::NO_UNSUBSCRIBE;
}
void GenericThread::onCycleEnd()
{
	/*@todo Esto es por compatibilidad. Mejorarlo cuando este listo lo del empty thread
	la idea final es que no se aporte ninguna funci�n de thread y todo sea e nbase a lanzar tareas y que se quede dormido cuando no haya. Requisitos:
	 - los Process han de avisar de alguna forma cuando est�n en wait para no parar el hilo si hay alguno pendiente-
	 - si un Process recibe un wakeup tiene que avisar el hilo
	*/
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
		//count = getPendingTaskCount();
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
	/*
	bool end;
	if ( !mEnd )
	{
no vale ya que el processTask viene despues y puede haber un mEnd
creo que esta funcion deber�a mandar siempre, por lo que si se hace desde fuera un mEnd, que esta lo cambie

deber�a avisar a la function de si se pide terminacion
			end = (*mThreadFunction)( this );
			mEnd = (mEnd || end); //maybe mEnd is changed inside threadfunction
		}*/
}