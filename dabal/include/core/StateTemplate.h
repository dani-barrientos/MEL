#pragma once

#include <core/State.h>
#include <core/Callback.h>
#include <list>
using std::list;

namespace core
{
	template<class T>
	class  StateTemplate : public State
	{
	private:
		//signature por transition functions. If return true, then exite, else, continue executing change function until true
		typedef bool (T::*ChangeStateFunction)( unsigned int milliseconds);
		typedef void (T::*StateFunction)( unsigned int milliseconds);
	public:
		StateTemplate():
			mInstance(0),
			mStateEntry( 0 ),
			mStateExit( 0 ),
			mState(0),
			mPhase(NONE)
			/*,mEvents( 0 )*/
			{};
		/**
		 * sets the three functions associated with a state ( entry, exit and state )
		 * 
		 * @param[in] instance. Pointer to object with state functions
		 * @param[in] stateEntre. Function to execute on satte enter
		 * @param[in] 
		 */
		void set(T* instance, ChangeStateFunction stateEntry, 
				ChangeStateFunction stateExit, 
				StateFunction state);
		/**
		* overridden from State
		 * called by StateMachine on state entering 
		 * 
		 */
		bool _executeEnterState(unsigned int milliseconds);
		/**
		 * overridden from State
		 * @param milliseconds    milliseconds
		 */
		bool _executeExitState(unsigned int milliseconds);
		/**
		 * overridden from State
		 * @param milliseconds    milliseconds
		 */
		void _executeState(unsigned int milliseconds);

	protected:
		/**
		 * object owner
		 */
		T* mInstance;
		/**
		 * pointer to member function to execute when state begins
		 */
		ChangeStateFunction mStateEntry;
		/**
		 * member function pointer to execute when machine exit state
		 */
		ChangeStateFunction mStateExit;
		/**
		 * state member function
		 */
		StateFunction mState;

		enum EPhase {NONE,ENTRY,EXIT,STATE} mPhase;
		inline EPhase getPhase() const{ return mPhase;};
	private:
		
		/**
		* events to call when time is elapsed
		* signature is: bool (unsigned int, State*):
		*			return bool. true if you want to unsubscribe
		*			param unsigned int. milliseconds
		*			State*: State which produced event
		*		
		*/
/*
@todo pensarlo bien
struct TimeElapsedEvent : public Event<unsigned int, State*>
		{
			unsigned int mPeriod;
			unsigned int mInitTime;
		};

		list< TimeElapsedEvent* >*	mEvents;

		void executeEvents( unsigned int );
		void deleteEvents( );
		*/
	};
template <class T>
bool StateTemplate<T>::_executeEnterState(unsigned int milliseconds)
{
	mInitialTime = milliseconds;
	mPhase = ENTRY;
	if ( mStateEntry )
	{
		return (mInstance->*mStateEntry)( milliseconds );
	}
	return true;
}
template <class T>
bool StateTemplate<T>::_executeExitState(unsigned int milliseconds)
{
	mElapsedTime = milliseconds - mInitialTime;
	mPhase = EXIT;
	if ( mStateExit )
	{
		return (mInstance->*mStateExit)( milliseconds );
	}
	return true;

}
template <class T>
void StateTemplate<T>::_executeState( unsigned int milliseconds )
{
	mPhase = STATE;
	mElapsedTime  = milliseconds  - mInitialTime;
	(mInstance->*mState)( milliseconds );
	//ejecutamos eventos

	/*if( mEvents )
	{
		executeEvents( milliseconds );
	}*/

}
	template <class T>
	void StateTemplate<T>::set(T* instance, ChangeStateFunction stateEntry, 
			ChangeStateFunction stateExit, 
			StateFunction state)
	{
		mInstance = instance;
		if ( stateEntry )
		{
			mStateEntry = stateEntry;
		}
		if ( stateExit )
		{
			mStateExit = stateExit;
		}
		mState = state;
	}
	/*
	template <class T>
	void StateTemplate<T>::executeEvents( unsigned int time ) 
	{	
		list<TimeElapsedEvent*>::iterator i = mEvents->begin();
		while ( i != mEvents->end() )
		{
			if ( milliseconds - (*i)->mInitTime ) > mEvent->mPeriod )
			{
				if ( (*(*i))( time, this ) )
				{
					mEvents->erase( i );
					delete *i;
				}else
				{
					++i;
				}
			}

			
		}
	}
	template <class T>
	void StateTemplate<T>::deleteEvents( )
	{
		if ( mEvents )
		{
			for ( list< TimeElapsedEvent* >::iterator i = mEvents->begin(); i != mEvents->end(); ++i )
			{
				delete *i;
			}
			delete mEvents;
			mEvents = 0;
		}
	}*/
}

