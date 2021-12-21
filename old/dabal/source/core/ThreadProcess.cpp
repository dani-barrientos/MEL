#include <core/ThreadProcess.h>
using core::ThreadProcess;

#include <core/Thread.h>
using core::Thread;

#include <mpl/ReturnAdaptor.h>
using mpl::returnAdaptor;

#include <mpl/MemberEncapsulate.h>
using mpl::makeMemberEncapsulate;

const unsigned short ThreadProcess::TIME_SLICE = 20;
ThreadProcess::ThreadProcess( Thread* thread ) :
	mThread( thread )
{
	setPeriod( TIME_SLICE );
}
ThreadProcess::ThreadProcess() :
	mThread( 0 )
{	
	setPeriod( TIME_SLICE );
	setProcessFunction( makeMemberEncapsulate( &ThreadProcess::emptyUpdate, this ));
};
ThreadProcess::~ThreadProcess()
{
	delete mThread;
}

void ThreadProcess::initConstruction()
{
	mYieldDone = true;
	mThread->subscribeFinishEvent( 
		
		addParam<void, Runnable*, void >
		(
			makeMemberEncapsulate( &ThreadProcess::threadFinished, this )
		)
	);
}
void ThreadProcess::update(uint64_t milliseconds )
{
	//sends thread a yield each period milliseconds
	//this is only orientative
	if ( mYieldDone && getState() != PAUSED )
	{
		//forma chapucera pero eficiente, dado que no se requiere precision
		//para evitar stack-overflow en mThread por no poder consumir todos los yield
		//primero comprobamos se se consumió el previo.
		mYieldDone = false;
		mThread->post(
			RUNNABLE_CREATETASK
			(
				returnAdaptor<void>
				(
					makeMemberEncapsulate( &ThreadProcess::yieldTask, this ),
					true
				)
			), 
			Runnable::NORMAL_PRIORITY_TASK
		); 
	}
}
void ThreadProcess::yieldTask()
{
	//@todotengo la impresion de que no vale pa mucho mirando el administrador de tareas..revisar
	Thread::yield();
	mYieldDone = true;
}
void ThreadProcess::onInit(uint64_t msegs)
{
	mThread->start();
}
bool ThreadProcess::onKill()
{
	mThread->finish();
	if ( mThread->finished() )
	{
		//TODO esto no es muy seguro. hay que revisarlo bien. No hago el join para no parar.¿hacerlo ahora?
		return true;
	}
	else
	{
		/* PROBLEMA: INTERBLOQUEO SI MAINLOOP ESPERA A QUE FINALIZA UN PROCESO SUYO

		//post task to do kill, in order to not depend on scheduler where this Process is inserted
		//in order to not cause an interblocking (for example, scheduler waiting for a ThreadProcess to kill
		//but this will depend on scheduler)
		//TODO cuidado, que pasaría si se hace el onKill desde el propio hilo y desde el scheduler....
		mThread->post(
			RUNNABLE_CREATETASK(
				returnAdaptor<void>
				(
					makeMemberEncapsulate( &Process::kill, (Process*)this )
					,true
				)
			)
		);MAAAAAL*/
		return false;
	}
}
void ThreadProcess::pause( bool resetPreviousTime )
{
	Process::pause();
	mThread->suspend();
}
void ThreadProcess::activate()
{
	Process::activate();
	if ( mThread->getState() == ::core::THREAD_SUSPENDED )
	{
		mThread->resume();
	}
	/*
	esto no vale porque es Process quien tiene que activarlo
	if ( mThread->getState() == ::core::THREAD_SUSPENDED )
	{
		mThread->resume();
	}else if ( mThread->getState() == ::core::THREAD_INIT )
	{
		mThread->start();
	}
	*/
}


void ThreadProcess::threadFinished(  )
{
	kill();
}

void ThreadProcess::join( unsigned int millis )
{
	getThread()->join();
}
