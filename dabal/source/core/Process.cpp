	
#include <core/Process.h>
#include <core/ProcessScheduler.h>

#include <stdlib.h>

using core::Process;
using core::ProcessScheduler;
using core::MThreadAttributtes;

#undef max
#include <limits>
#include <assert.h>

DABAL_CORE_OBJECT_TYPEINFO_IMPL_ROOT( Process );

/**
* =============================================================================
*
* @param tipo
*/
Process::Process( bool reserveStack,unsigned short capacity  )
	: mLastTime(0),
	mPeriod(0),
	mFinished(false),
	mPreviousTime(0),
	mResetPreviousTime(false),	
	mProcessId(0),
	mOwnerProcessScheduler( 0 ),
	//mExecuteNextAfterFinish(false),
	mState(PREPARED),
	mSleeped(false),
	mWaiting(false),
	mWakeup(false),
	mExtrainfo(0),
	mStartTime(0),
	mPausedTime(0)
	//,mNext( NULL )
	//,mKillCallback(0)
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
	//attached paused
	if ( mState == PAUSED )
		mPauseTime = mOwnerProcessScheduler->getTimer()->getMilliseconds();
}	




/**
* pause this process
*/
void Process::pause( bool resetPreviousTime )
{
	if ( mState != PAUSED )
	{
		mPreviousState = mState;
		mState = PAUSED;
		mResetPreviousTime = resetPreviousTime;

		//if process is not atached to scheduler, doesn't matter pause time
		if ( mOwnerProcessScheduler )
			mPauseTime = mOwnerProcessScheduler->getTimer()->getMilliseconds();
	}
}
/**
* activate this process
*/
void Process::activate(  )
{
	if( mState == PAUSED )
	{
		mState = mPreviousState;
		if ( mOwnerProcessScheduler )
		{
			uint64_t currTime = mOwnerProcessScheduler->getTimer()->getMilliseconds();
			mPausedTime += (unsigned int)(currTime-mPauseTime); //I supposed pause time won't be more thant unsigned int..
		}
		mPauseTime = 0;
	}
}

void Process::reset()
{
	mState = PREPARED;
	mLastTime = (unsigned int)((mOwnerProcessScheduler!=NULL)?mOwnerProcessScheduler->getTimer()->getMilliseconds():0);
	mPreviousTime = 0;
}


Process::EProcessState Process::getState() const
{

	return  mState;
}


/**
* execution function. It calls update() when time > mPeriod
*
* @param msegs    msegs
*/
void Process::onUpdate(uint64_t msegs)
{
	if( mSleeped )
		return; 
	unsigned int lap = (unsigned int)((msegs-mLastTime)-mPausedTime);
	//TODO se est� enrevesando ya esta comparaci�n. Revisarla para simplificarla si se puede
	unsigned int mask = PREPARED | PREPARED_TO_DIE/* | TRYING_TO_KILL*/;
	if ( mState == TRYING_TO_KILL )
	{
		//call kill again
		kill();
	}
	//TODO creo que esto es un fallo conceptual importante..ya que llama al update tanto si se cumple el per�odo como si se est� TRYING_TO_KILL. No parece que tenga sentido
	if (  ( mState == PREPARED && lap >= mStartTime ) ||
		( !(mState & mask) && lap >= mPeriod ) ) //TODO tal vez tenga sentido que si est� TRYING_TO_KILL y cumple el periodo si lo ejecute
	{
		mUpdateTime = msegs;
		/*mPausedTime = 0; duda, para calcular el elapsed time lo necesitaria, pero no s� si vale hacerlo despu�s
				SI EL checkMicrothread HICISE OTRO WAIT, CREO QUE NO FURRULARIA*/
		checkMicrothread( msegs ); 
		mPausedTime = 0;
		mLastTime = (unsigned int)mOwnerProcessScheduler->getTimer()->getMilliseconds();
		mPreviousTime = msegs;

	}
	//los procesos asociados se ejecutan independientemente de que este proceso entre en ejecuci�n
	/*if ( mAttachedProcesses )
	{
		mAttachedProcesses->executeProcesses();		
	}*/
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
void Process::setFinished(bool value)
{

	mFinished = value;
}



/**
* main execution block. Normally it will not be redefined. 
*
*
* @param msegs    msegs
*/
void Process::execute(uint64_t msegs){

	switch ( mState )
	{
	case PREPARED:
		mFinished = false;
		//ya que en este momento ya tiene que estar creado el temporizador
		//mLastTime = msegs;
		mPreviousTime = msegs;
		mBeginTime = msegs;
		mState = INITIATED;
		onInit( msegs );   //por si se quiere hacer alguna inicializacion
		update( msegs );  //maybe update wants change state (through kill, for example)

		break;
	case INITIATED:
	case TRYING_TO_KILL:
		//llamamos a la funci�n de comportamiento del objeto particular
		//TODO no s� si esto est� bien colocado del todo aqui. La pretensi�n es
		//que no corra el tiempo si no se desea al pausar
		if ( mResetPreviousTime )
		{
			mPreviousTime = msegs;
			mResetPreviousTime = false;
		}
		update( msegs );
		break;
	case WAITING_FOR_SCHEDULED:
//		TEMAS: �REDUCIR PER�ODO?�avisar de alguna forma?
		//process was in "switched" state. Now It can be killed
		KillEventSubscriptor::triggerCallbacks( shared_from_this() );
		mState = PREPARED_TO_DIE;
		killDone();
		break;
    default:
            ;
	}
}

unsigned int Process::getElapsedTime() const
{
	return (unsigned int)(mOwnerProcessScheduler->getTimer()->getMilliseconds() - (uint64_t)mUpdateTime);
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
		_switchProcess(); //@todo quitar este par�metro del _switchProcess

		p->setPeriod(currentPeriod); //siempre restauramos por si acaso se despert� por wakeup
		if (p->getState() == WAITING_FOR_SCHEDULED)
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
	p->mSleeped = true;
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
	return result;
}
Process::ESwitchResult Process::_wait( unsigned int msegs, Callback<void,void>* postWait ) 
{
	auto p = ProcessScheduler::getCurrentProcess();
	unsigned int currentPeriod = p->getPeriod();
	p->setPeriod( msegs );
	p->mWaiting = true;  
	//trigger callback
	if ( postWait )
	{
		(*postWait)();
		delete postWait;
	}
	ESwitchResult result = switchProcess( false );		
	p->setPeriod( currentPeriod );
	p->mWaiting = false;
	return result;
}
void Process::wakeUp()
{
    if ( mSleeped || mSwitched ) 
    {
		if ( mSleeped )
			//notify scheduler
			mOwnerProcessScheduler->processAwakened( shared_from_this() );
        setPeriod( 0 );
        mSleeped = false;
        mWakeup = true;        
    }
}
