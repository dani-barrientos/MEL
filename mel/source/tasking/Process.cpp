	
#include <tasking/Process.h>
#include <tasking/ProcessScheduler.h>
#include <stdlib.h>
using mel::tasking::Process;
using mel::tasking::ProcessScheduler;
using mel::tasking::MThreadAttributtes;
#include <exception>
#undef max
#include <limits>
#include <assert.h>


Process::Process( unsigned short capacity  )
	: 
	mPeriod(0),
	mPauseReq(false),
	mLastUpdateTime(0),
	mOwnerProcessScheduler( 0 ),
	mState(EProcessState::PREPARED),
	mWakeup(false)

{
	mSwitched = false;
    mStackSize = 0;
	if ( capacity > 0 )
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

void Process::update(uint64_t msegs)
{	
	checkMicrothread( msegs ); 
	mLastUpdateTime = msegs;
	//@todo mirar bien la diferencia entre estos dos. Supongo que este mLastTime es diferente cuando hay cambios de contexto??? en principio no..
//	mLastTime = (unsigned int)mOwnerProcessScheduler->getTimer()->getMilliseconds();
	//mPreviousTime = msegs;
}

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
		sleep();
	}
	switch ( mState )
	{
	case EProcessState::PREPARED:
		mState = EProcessState::INITIATED;
		onInit( msegs );   	
	case EProcessState::INITIATED:
	case EProcessState::TRYING_TO_KILL:		
		onUpdate( msegs );
		break;
	case EProcessState::KILLING_WAITING_FOR_SCHEDULED:
		//process was in "switched" state. Now It can be killed
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


void mel_tasking_resizeStack(   MThreadAttributtes* process,  unsigned int newSize )
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
	return _postWait(msegs,_preWait());	
//	return _wait( msegs, NULL );
}
Process::ESwitchResult Process::switchProcess( bool v )
{
	ESwitchResult result;
	auto p = ProcessScheduler::getCurrentProcess();
	auto state = p->getState();
	if (state == EProcessState::PREPARED_TO_DIE || state == EProcessState::DEAD || state == EProcessState::KILLING_WAITING_FOR_SCHEDULED)
		result = ESwitchResult::ESWITCH_KILL;
	else
	{
		#ifdef MEL_WINDOWS
		if (std::current_exception() != nullptr)
			mel::text::warn("Process::switchProcess. Doing context switch while pending exception can lead to crashes. You should avoid Process context switches on 'catch' contexts");
		#endif
		unsigned int currentPeriod = p->getPeriod();
		if (v)
			p->setPeriod(0);
		_switchProcess();

		if ( p->mPauseReq && !p->mWakeup )  //maybe a pause was  requested while switched
		{
			p->mPauseReq = false;
			sleep();
			
		}else
		{
			p->setPeriod(currentPeriod); //siempre restauramos por si acaso se despertó por wakeup
			if (p->getState() == EProcessState::KILLING_WAITING_FOR_SCHEDULED)
				result = ESwitchResult::ESWITCH_KILL;
			else if (p->mWakeup)
			{
				result = ESwitchResult::ESWITCH_WAKEUP;
			}
			else
				result = ESwitchResult::ESWITCH_OK;
		}
		p->mWakeup = false;
	}
	return result;
}
mel::mpl::Tuple<TYPELIST(bool,Process*)> Process::_preWait() 
{
	auto p = ProcessScheduler::getCurrentProcess();
	
	auto prevSwitch = p->mSwitched;
	p->mSwitched = true; //needed to cheat that is already switched just in case is checked as a response of postSleep
	return mel::mpl::Tuple<TYPELIST(bool,Process*)>(prevSwitch,p.get());
	
}
Process::ESwitchResult Process::_postWait(uint64_t msegs,mel::mpl::Tuple<TYPELIST(bool,Process*)> input) 
{
	ESwitchResult result;
	auto p = input.get<1>();
	unsigned int currentPeriod = p->getPeriod();
	p->setPeriod( msegs );
	if (!input.get<0>())//!no multithread safe!!
	{
		result = switchProcess(false);
		if ( result == ESwitchResult::ESWITCH_KILL)
			p->mSwitched = false;
	}
	else {
		result = ESwitchResult::ESWITCH_OK;
	}
	p->setPeriod( currentPeriod );
	return result;
}
Process::ESwitchResult Process::sleep( )
{
	return _postSleep(_preSleep());
}


//return: 0 -> false, 1 = true; 2 = already asleep, 3 = process killed 
mel::mpl::Tuple<TYPELIST(int,Process*,unsigned int)> Process::_preSleep()
{
	auto p = ProcessScheduler::getCurrentProcess();
	const auto state = p->getState();
	if ( state == EProcessState::ASLEEP  )
	{
		return mel::mpl::Tuple<TYPELIST(int,Process*,unsigned int)>(2,p.get(),0);
	}else if (state == EProcessState::PREPARED_TO_DIE || state == EProcessState::DEAD || state == EProcessState::KILLING_WAITING_FOR_SCHEDULED)
	{
		return mel::mpl::Tuple<TYPELIST(int,Process*,unsigned int)>(3,p.get(),0);
	}
	
	p->mPreviousState = state;
	p->mState = EProcessState::ASLEEP;
	p->mOwnerProcessScheduler->processAsleep(p);	
	unsigned int currentPeriod = p->getPeriod();
	p->setPeriod( std::numeric_limits<unsigned int>::max() ); //maximum period
	auto prevSwitch = p->mSwitched;
	p->mSwitched = true; //needed to cheat that is already switched just in case is checked as a response of postSleep
	return mel::mpl::Tuple<TYPELIST(int,Process*,unsigned int)>(prevSwitch?1:0,p.get(),currentPeriod);
}
Process::ESwitchResult Process::_postSleep(mel::mpl::Tuple<TYPELIST(int,Process*,unsigned int)> input) 
{
	auto p = input.get<1>();
	if ( input.get<0>() == 2 ) //it hasn't any sense, but just in case this condition could be reached
	{
		return ESwitchResult::ESWITCH_ERROR;
	}else if ( input.get<0>() == 3 )
	{
		//@todo creo que este caso no l oestoy manejando
		p->mSwitched = false;
		return ESwitchResult::ESWITCH_KILL;
	}
	ESwitchResult result;
	if (!input.get<0>())//!no multithread safe!!
	{
		result = switchProcess(false);
		if ( result == ESwitchResult::ESWITCH_KILL)
			p->mSwitched = false;
	}
	else
	{
		result = ESwitchResult::ESWITCH_OK;
	}
	auto currentPeriod = input.get<2>();
	p->setPeriod( currentPeriod );
	p->mState = p->mPreviousState;	
	return result;
}

void Process::wakeUp()
{
	//@todo termina verificar que está pausado y tal y poner estado anterior y todo eso..
	if ( mSwitched || mState == EProcessState::ASLEEP) 
	{	
		//notify scheduler
		if( mState == EProcessState::ASLEEP )
			mOwnerProcessScheduler->processAwakened( shared_from_this() ); 
		setPeriod( 0 );
	//	mState = mPreviousState; //creo que esto no debe ser aqui!!! LO HICE PORQUE EL SCHEDULER NO TENIA EN CUENTA SI SE HABÁI DESPERTADO
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
					mState = EProcessState::KILLING_WAITING_FOR_SCHEDULED;
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
						mState = EProcessState::KILLING_WAITING_FOR_SCHEDULED;
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