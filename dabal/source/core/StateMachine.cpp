///////////////////////////////////////////////////////////
//  StateMachine.cpp
//  Implementation of the Class StateMachine
//  Created on:      29-mar-2005 10:00:14
///////////////////////////////////////////////////////////

#include <core/StateMachine.h>
using core::StateMachine;
using core::State;

StateMachine::StateMachine(void)
    : mCurrentState(0),
	  mNewState(0),
	  mOldState(0),
	  mExecutionState(INITIAL)
{

	mInitialState.set(this,&StateMachine::enterInitialState, 
			&StateMachine::exitInitialState,
			&StateMachine::initialStateFunction);
	mCurrentState= &mInitialState;
}

StateMachine::~StateMachine(void)
{

}

void StateMachine::update(unsigned int milliseconds)
{
	//TODO el codigo esta bastante chapuceado pero es por temas de eficiencia
	switch( mExecutionState )
	{
	case INITIAL:
		//@todo chaupza temporal para evitar posible problema con el INIT + microhilos
		mExecutionState = NORMAL;
		break;
	case NORMAL:
		if ( mNewState )
		{
			StateChangeSubscriptor::triggerCallbacks( this, mCurrentState,SC_PRE_EXIT );
			if ( mCurrentState->executeExitState( milliseconds ) )
			{
				StateChangeSubscriptor::triggerCallbacks( this, mCurrentState,SC_POST_EXIT );
				mCurrentState = mNewState;
				mNewState = 0;  //reinicia el nuevo estado
				mCurrentState->setStateMachine( this );
				StateChangeSubscriptor::triggerCallbacks( this, mCurrentState,SC_PRE_ENTRY );
				if ( mCurrentState->executeEnterState( milliseconds, mOldState ) ) //this function maybe will do a gotoState, so change mNewState!
				{
					StateChangeSubscriptor::triggerCallbacks( this, mCurrentState,SC_POST_ENTRY );
					if ( !mNewState ) //because maybe executeEnterState did a gotoState, so i don't want to execute it's update
					{
						mCurrentState->executeState( milliseconds ); 
					}
					mExecutionState = NORMAL;
				}else
				{
					mExecutionState = ENTERING;
				}
			}else
			{
				mExecutionState = EXITING;
			}

		}else
		{
			mCurrentState->executeState( milliseconds ); 
		}

		break;
	case EXITING:
		if ( mCurrentState->executeExitState( milliseconds ) )
		{
			StateChangeSubscriptor::triggerCallbacks( this, mCurrentState,SC_POST_EXIT );
			//@todo ¿hacer mCurrentState->setStateMachine( NULL ) ?
			mCurrentState = mNewState;
			mNewState = 0;  //reinicia el nuevo estado

			mCurrentState->setStateMachine( this );
			StateChangeSubscriptor::triggerCallbacks( this, mCurrentState,SC_PRE_ENTRY );
			if ( mCurrentState->executeEnterState( milliseconds,mOldState  ) ) //this function maybe will do a gotoState, so change mNewState!
			{
				StateChangeSubscriptor::triggerCallbacks( this, mCurrentState,SC_POST_ENTRY );
				if ( !mNewState ) //because maybe executeEnterState did a gotoState, so i don't want to execute it's update
				{
					mCurrentState->executeState( milliseconds ); 
				}
				mExecutionState = NORMAL;
			}else
			{
				mExecutionState = ENTERING; //continue exiting until return true
			}

		}
		break;
	case ENTERING:
		if ( mCurrentState->executeEnterState( milliseconds, mOldState ) ) //this function maybe will do a gotoState, so change mNewState!
		{
			StateChangeSubscriptor::triggerCallbacks( this, mCurrentState,SC_POST_ENTRY );
			if ( !mNewState ) //because maybe executeEnterState did a gotoState, so i don't want to execute its update
			{
				mCurrentState->executeState( milliseconds ); 
			}
			mExecutionState = NORMAL;
		}
		break;
	};

}
void StateMachine::reset()
{
	Process::reset();
	mCurrentState = &mInitialState;
}
