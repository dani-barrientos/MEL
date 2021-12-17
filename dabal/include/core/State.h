///////////////////////////////////////////////////////////
//  State.h
//  Implementation of the Class State
//  Created on:      29-mar-2005 10:00:13
///////////////////////////////////////////////////////////


#pragma once

#include <FoundationLibType.h>
#include <cstdint>

namespace core
{
	class StateMachine; //predeclaration
	/**
	* @class State
	* base class for StateMachine states. It's strongly related to StateTemplate and is it's Interface, so you
	* must use StateTemplte to create concrete states
	*/

	class FOUNDATION_API State 
	{
	public:
		State(void);
		virtual ~State(void);
		/**
		 * function describing action to execute when machine enters state
		 * 
		 * @param milliseconds current millis
		 * @param source source state the StateMachine is coming from
		 * @return `true` if exit succeedes and the enter stat is finished; `false` if it still needs to continue executing
		 */
		inline bool executeEnterState(uint64_t milliseconds, State* source );
		/**
		 * function describing the action to execute when machine exit state
		 * 
		 * @param milliseconds current millis
		 * @return `true` if the state has actually fionished; `false` if still needs to continue executing
		 */
		inline bool executeExitState(uint64_t milliseconds);
		/**
		 * state function
		 * 
		 * @param milliseconds current millis
		 */
		inline void executeState(uint64_t milliseconds);
		/**
		 * gets elapsed time in this state ( in chunks of actualization time )
		 */
		inline uint64_t getElapsedTime() const;
		/**
		 * time at state begin
		 */
		inline uint64_t getInitialTime() const;
		inline void resetElapsedTime();

		/**
		* gets owner state machine
		*/
		inline StateMachine* getStateMachine() const;

		/**
		* sets owner statemachine. Intended to be used by StateMachine
		*/
		inline void setStateMachine( StateMachine* );

		inline State* getSourceState() const;
	protected:
		/**
		 * time at state begin
		 */
		uint64_t mInitialTime;
		/**
		 * holds elapsed time since state begin
		 */
		uint64_t mElapsedTime;
		
		virtual bool _executeEnterState(uint64_t milliseconds) = 0;
		/**
		* function describing the action to execute when machine exit state
		* 
		* @param milliseconds current millis
		* @return `true` if the state has finished; `false` if still needs to continue executing
		*/
		virtual bool _executeExitState(uint64_t milliseconds) = 0;
		/**
		* state function
		* 
		* @param milliseconds  current millis
		*/
		virtual void _executeState(uint64_t milliseconds) = 0;

	private:
		StateMachine*	mOwner;
		State*			mSource;

	};

//inlines
uint64_t State::getElapsedTime() const
{
	return mElapsedTime;
}
uint64_t State::getInitialTime() const
{
	return mInitialTime;
}
void State::resetElapsedTime()
{
	mInitialTime = mElapsedTime;
}
StateMachine* State::getStateMachine() const
{
	return mOwner;
}
void State::setStateMachine( StateMachine* sm )
{
	mOwner = sm;
}
bool State::executeEnterState(uint64_t milliseconds, State* source)
{
	mSource = source;
	return _executeEnterState( milliseconds );
}
bool State::executeExitState(uint64_t milliseconds)
{
	return _executeExitState( milliseconds );
}
void State::executeState(uint64_t milliseconds)
{
	return _executeState( milliseconds );
}
State* State::getSourceState() const
{
	return mSource;
}

}

