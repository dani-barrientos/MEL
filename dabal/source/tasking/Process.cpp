	
#include <tasking/Process.h>
#include <tasking/ProcessScheduler.h>

#include <stdlib.h>

using tasking::Process;
using tasking::ProcessScheduler;
using tasking::MThreadAttributtes;

#undef max
#include <limits>
#include <assert.h>
#include <spdlog/spdlog.h>

DABAL_CORE_OBJECT_TYPEINFO_IMPL_ROOT( Process );

/**
* =============================================================================
*
* @param tipo
*/
Process::Process( bool reserveStack,unsigned short capacity  )
	: 
	//mLastTime(0),
	mPeriod(0),
	//mFinished(false),
	mPauseReq(false),
	//mPreviousTime(0),
	mLastUpdateTime(0),
	mProcessId(0),
	mOwnerProcessScheduler( 0 ),
	mState(EProcessState::PREPARED),
	mWakeup(false)
//	,mExtrainfo(0)
//	,mStartTime(0)
{
	mSwitched = false;
    mStackSize = 0;
	if ( reserveStack )
	{
        assert( (capacity & 3)==0 ); //TODO multiplo de 16 en GCC!!
        mCapacity = capacity;
        //mStack = new unsigned char[stackSize];
        //RESERVAR SI SE QUIERE EN CONSTRUCTOR
        mStack = (unsigned char*)malloc( mCapacity );
	}else
	{
	    mCapacity = 0;
        mStack = NULL;
	}
	//mActualSP = &(mStack[stackSize]);
	mStackEnd = mStack; //apunta al final ocupado
}

Process::~Process(void)
{

    free( (void*)mStack );
	/*if ( mAttachedProcesses )
	{
		delete mAttachedProcesses;
	}*/
	//delete mKillCallback;
}



/**
* sets processManager which holds this process
*
* @param gestor    gestor
*/
void Process::setProcessScheduler(ProcessScheduler* const gestor)
{
	mOwnerProcessScheduler = gestor;
}	
void Process::pause( )
{
	mPauseReq = true;// atomico?? no quiero que la lectura sea atómica	
	onPause();
}

void Process::reset()
{
	mState = EProcessState::PREPARED;
	mLastUpdateTime = 0;
	//mLastTime = (unsigned int)((mOwnerProcessScheduler!=NULL)?mOwnerProcessScheduler->getTimer()->getMilliseconds():0); ??
//	mPreviousTime = 0;
}


/**
* execution function. It calls update() when time > mPeriod
*
* @param msegs    msegs
*/
// void Process::update(uint64_t msegs)
// {
// 	if( mSleeped )
// 		return; 
// 	unsigned int lap = (unsigned int)((msegs-mLastTime)-mPausedTime);
// 	//TODO se est� enrevesando ya esta comparaci�n. Revisarla para simplificarla si se puede
// 	auto mask = EProcessState::PREPARED | EProcessState::PREPARED_TO_DIE/* | TRYING_TO_KILL*/;
// 	if ( mState == EProcessState::TRYING_TO_KILL )
// 	{
// 		//call kill again
// 		kill();
// 	}
// 	//TODO creo que esto es un fallo conceptual importante..ya que llama al update tanto si se cumple el per�odo como si se est� TRYING_TO_KILL. No parece que tenga sentido
// 	if (  ( mState == EProcessState::PREPARED && lap >= mStartTime ) ||
// 		( !(mState & mask) && lap >= mPeriod ) ) //TODO tal vez tenga sentido que si est� TRYING_TO_KILL y cumple el periodo si lo ejecute
// 	{
// 		mUpdateTime = msegs;
// 		/*mPausedTime = 0; duda, para calcular el elapsed time lo necesitaria, pero no s� si vale hacerlo despu�s
// 				SI EL checkMicrothread HICISE OTRO WAIT, CREO QUE NO FURRULARIA*/
// 		checkMicrothread( msegs ); 
// 		mPausedTime = 0;
// 		mLastTime = (unsigned int)mOwnerProcessScheduler->getTimer()->getMilliseconds();
// 		mPreviousTime = msegs;

// 	}
// 	// //los procesos asociados se ejecutan independientemente de que este proceso entre en ejecuci�n
// 	// if ( mAttachedProcesses )
// 	// {
// 	// 	mAttachedProcesses->executeProcesses();		
// 	// }
// }
void Process::update(uint64_t msegs)
{	
	checkMicrothread( msegs ); 
	mLastUpdateTime = msegs;
	//@todo mirar bien la diferencia entre estos dos. Supongo que este mLastTime es diferente cuando hay cambios de contexto??? en principio no..
//	mLastTime = (unsigned int)mOwnerProcessScheduler->getTimer()->getMilliseconds();
	//mPreviousTime = msegs;
}
//void Process::attachProcess( Process* p )
//{
//	if ( !mAttachedProcesses )
//	{
//		mAttachedProcesses = new ProcessScheduler;
//		mAttachedProcesses->ini();
//		mAttachedProcesses->setTimer( mOwnerProcessScheduler->getTimer() );
//	}
//	mAttachedProcesses->insertProcess( p,ProcessScheduler::NORMAL );
//}




/**
* indicates this process that was succesfully finished
*/
// void Process::setFinished(bool value)
// {

// 	mFinished = value;
// }



/**
* main execution block. Normally it will not be redefined. 
*
*
* @param msegs    msegs
*/
void Process::_execute(uint64_t msegs)
{
	if ( mPauseReq)
	{
		mPauseReq = false;
		mState = EProcessState::ASLEEP;
	}
	switch ( mState )
	{
	case EProcessState::PREPARED:
		//mFinished = false;
		//mPreviousTime = msegs;
		//mBeginTime = msegs;
		mState = EProcessState::INITIATED;
		onInit( msegs );   	
	case EProcessState::INITIATED:
	case EProcessState::TRYING_TO_KILL:		
		onUpdate( msegs );
		break;
	case EProcessState::WAITING_FOR_SCHEDULED:
		//process was in "switched" state. Now It can be killed
		//KillEventSubscriptor::triggerCallbacks( shared_from_this() );
		mState = EProcessState::PREPARED_TO_DIE;
		killDone();
		break;
    default:
            ;
	}
}

unsigned int Process::getElapsedTime() const
{
	return (unsigned int)(mOwnerProcessScheduler->getTimer()->getMilliseconds() - (uint64_t)mLastUpdateTime);
}


//void Process::setNext( Process* p )
//{
//	if ( !mNext )
//	{
//		mNext = p;
//	}else
//	{
//		mNext->setNext( p );
//	}
//}


void resizeStack(   MThreadAttributtes* process,  unsigned int newSize )
{
	if (process->mCapacity< newSize )
	{
		free( (void*)process->mStack );
		process->mStack = (unsigned char*)malloc( newSize );
		process->mCapacity = newSize;
	}

	process->mStackEnd = &(process->mStack[newSize]);
	process->mStackSize = newSize;
}

Process::ESwitchResult Process::wait( unsigned int msegs )
{
	return _wait( msegs, NULL );
}
Process::ESwitchResult Process::switchProcess( bool v )
{
	ESwitchResult result;
	auto p = ProcessScheduler::getCurrentProcess();
	auto state = p->getState();
	if (state == EProcessState::PREPARED_TO_DIE || state == EProcessState::DEAD)
		result = ESwitchResult::ESWITCH_KILL;
	else
	{
		unsigned int currentPeriod = p->getPeriod();
		if (v)
			p->setPeriod(0);
		_switchProcess();
		p->setPeriod(currentPeriod); //siempre restauramos por si acaso se despertó por wakeup
		if (p->getState() == EProcessState::WAITING_FOR_SCHEDULED)
			result = ESwitchResult::ESWITCH_KILL;
		else if (p->mWakeup)
		{
			result = ESwitchResult::ESWITCH_WAKEUP;
		}
		else
			result = ESwitchResult::ESWITCH_OK;
		p->mWakeup = false;
	}
	return result;
}
Process::ESwitchResult Process::sleep( )
{
	return _sleep( NULL );
}

Process::ESwitchResult Process::_sleep( Callback<void,void>* postSleep )
{
	ESwitchResult result;
	auto p = ProcessScheduler::getCurrentProcess();
	const auto state = p->getState();
	if ( state == EProcessState::ASLEEP ) //it hasn't any sense, but just in case this condition could be reached
	{
		spdlog::warn("Process_sleep: process is already asleep");
		return ESwitchResult::ESWITCH_ERROR;
	}
	p->mPreviousState = state;
	p->mOwnerProcessScheduler->processAsleep(p);
	unsigned int currentPeriod = p->getPeriod();
	p->setPeriod( std::numeric_limits<unsigned int>::max() ); //maximum period
	//trigger callback
	if ( postSleep )
	{
		(*postSleep)();
		delete postSleep;
	}
	if (!p->mSwitched) {//!no multithread safe!!
		result = switchProcess(false);
	}
	else {
		result = ESwitchResult::ESWITCH_OK;
	}
	p->setPeriod( currentPeriod );
	p->mState = p->mPreviousState;
	return result;
}
Process::ESwitchResult Process::_wait( unsigned int msegs, Callback<void,void>* postWait ) 
{
	auto p = ProcessScheduler::getCurrentProcess();
	unsigned int currentPeriod = p->getPeriod();
	p->setPeriod( msegs );
	//trigger callback
	if ( postWait )
	{
		(*postWait)();
		delete postWait;
	}
	ESwitchResult result = switchProcess( false );		
	p->setPeriod( currentPeriod );
	return result;
}
void Process::wakeUp()
{
	//@todo termina verificar que está pausado y tal y poner estado anterior y todo eso..
	if ( mSwitched || mState == EProcessState::ASLEEP) 
	{	
		if ( mState == EProcessState::ASLEEP )
			//notify scheduler
			mOwnerProcessScheduler->processAwakened( shared_from_this() );
		setPeriod( 0 );
		mState = mPreviousState;
		mWakeup = true;        
		onWakeUp();
	}
}
void Process::kill( bool force )
	{
		//TODO esta funcion no est� un pijo bien, hay que revisarla junto con el Process::execute

		if ( mState != EProcessState::DEAD ) 
		{
			if ( force )
			{
				if ( mSwitched )
				{
					mState = EProcessState::WAITING_FOR_SCHEDULED;
					wakeUp();
				}else
				{
					mState = EProcessState::PREPARED_TO_DIE;
					killDone();
				}
			}else
			{
				bool ok = onKill();
				if ( ok )
				{
					if ( mSwitched )
					{
						mState = EProcessState::WAITING_FOR_SCHEDULED;
						wakeUp();
					}else
					{
						mState = EProcessState::PREPARED_TO_DIE;
						killDone();
					}
				}else
				{
					mState = EProcessState::TRYING_TO_KILL;
				}

			}
			if ( mState == EProcessState::PREPARED_TO_DIE )	
			{
			//	KillEventSubscriptor::triggerCallbacks( shared_from_this() );
			}

		}
	}