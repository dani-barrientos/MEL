///////////////////////////////////////////////////////////
//  StateMachine.h
//  Implementation of the Class StateMachine
//  Created on:      29-mar-2005 10:00:13
///////////////////////////////////////////////////////////
#pragma once

#include <core/State.h>
#include <core/StateTemplate.h>
#include <core/Process.h>
#include <core/CallbackSubscriptor.h>
#include <mpl/Int2Type.h>
using core::Process;

namespace core
{
	//subscriptor type for state change events
	enum EStateChangeStage{ SC_PRE_ENTRY,SC_POST_ENTRY, SC_PRE_EXIT,SC_POST_EXIT };
	typedef CallbackSubscriptor<::mpl::Int2Type<0>(),true,bool,StateMachine*,State*,EStateChangeStage> StateChangeSubscriptor;
	/**
	* Base StateMachine class.
	* It's a Process, so it's apdated every period milliseconds. It manages states (@see State)
	*
	*/
	class FOUNDATION_API StateMachine : public Process, public StateChangeSubscriptor
	{
	public:
		StateMachine(void);
		virtual ~StateMachine(void);
		/**
		 * updates state machine, executing state functions
		 * overridden from Process
		 * 
		 * @param milliseconds    update time
		 */
		void update(unsigned int milliseconds);
		inline State& getCurrentState();	
		inline const State& getCurrentState() const;	
		/**
		* Returns the state that is going to become the current one soon 
		*/
		inline State* getNewState();
		inline const State* getNewState() const;	
		/**
		 * check if machine is in passed state
		 * 
		 * @param state the state to check
		 */
		inline bool isInState(const State& state) const{

			if ( mCurrentState == &state )
			{
				return true;
			}else
				return false;
		}
		/**
		 * set machine in a new state
		 * 
		 * @param newState new state to go
		 * @remarks it's neccesary to call it to leave initial state
		 */
		 inline bool gotoState(State* newState);
		/**
		 * default (empty) entry function for initial state
		 * 
		 * @param milliseconds    milliseconds
		 */
		virtual bool enterInitialState(unsigned int milliseconds){return true;};
		/**
		 * default (empty) exit function for initial state
		 * 
		 * @param milisegundos current millis
		 * @return `true` if the initial estate exited; `false` it it still needs to continue executing
		 */
		virtual bool exitInitialState(unsigned int milisegundos){return true;};
		/**
		 * default (empty) function for the initial state
		 * 
		 * @param milliseconds    milliseconds
		 */
		virtual void initialStateFunction(unsigned int milliseconds){};

		/**
		* overridden from Process
		* set Machine in Initial state without execute previous state exitactions
		*/
		void reset();
		//! @see StateChangeSubscriptor
		template <class F> int subscribeStateChange( F functor )
		{
			return StateChangeSubscriptor::subscribeCallback( functor );
		}
		template <class F> void unsubscribeStateChange( F functor )
		{
			StateChangeSubscriptor::unsubscribeCallback( functor );
		}
	protected:
		/**
		 * state to transition in next update
		 */
		State*						mNewState;
		State*						mOldState;
		/**
		 * default state.
		 */
		StateTemplate<StateMachine>	mInitialState;
	private:
		enum {INITIAL,NORMAL,ENTERING,EXITING}		mExecutionState;
		core::State*				mCurrentState;

	};

	//INLINES
	State& StateMachine::getCurrentState()
	{
		return *mCurrentState;
	}
	const State& StateMachine::getCurrentState() const
	{
		return *mCurrentState;
	}
	State* StateMachine::getNewState()
	{
		return mNewState;
	}
	const State* StateMachine::getNewState() const
	{
		return mNewState;
	}
	bool StateMachine::gotoState( State* newState)
	{
		mOldState = mCurrentState;
		mNewState = newState; 
		return true;
	}



}

