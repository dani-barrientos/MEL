#pragma once

#include <core/Process.h>
#include <core/GenericThread.h>

#include <mpl/Linker.h>
using mpl::link1st;

namespace core
{
	class Thread; //predeclaration
	/**
	* @class ThreadProcess
	* @brief Special Process that manages a Thread. 
	*
	* The meaning of period here is the period used to make yield on the thread, so it's not a real period
	* By default this period is set to the typical time-slice
	*
	* ThreadProcess can be constructed 3 ways:
	*		-giving a Thread (see constructor details)
	*		-giving a functor ( bool f( ThreadProcess* ) ).
	*		-empty. You can set functor later with setProcessFunction
	*
	* In both modes, when the thread finish, this Process is killed, so you haven't to worry about
	* killing it
	*
	*/
	class DABAL_API ThreadProcess : public Process
	{
	public:
		/**
		* construct with thread. 
		* Process take ownership of thread, so yo shouln't:
		*		- start Thread It's done internally in the onInit function.
		*		- manipulate that thread directly
		*		- delete that thread
		* @remarks period is set to TIME_SLICE milliseconds, typical slice in threading, but you can change it later
		*/ 
		ThreadProcess( Thread* thread );
		/**
		* cosntructor with functor.
		* that functor has signature:
		*	bool f(ThreadProcess* tp,bool kill ), where:
		*		-param[in] tp. this ThreadProcess
		*		-param[in] kill. Kill request
		*		-return. True if want/can terminate

		* create internally a GenericThread that execute functor. 
		* @see GenericThread 
		* @remarks period is set to 20 milliseconds, typical slice in threading, but you can change it later
		*/
		template <class F>
			ThreadProcess( F functor );
		/**
		* default constructor. Create a thread with an empty functor. You can change it later
		*/
		ThreadProcess();
		~ThreadProcess();

		/**
		* set functor to execute.
		* You can change the functor given in constructor
		* @remarks chaguing the functor once it's stablished can get to unespected results. You must be sure
		* you task is finished before chaguing it
		*/
		template <class F> inline void setProcessFunction( F functor ); 

		inline Thread* getThread() const;
		/**
		* Pause the thread.
		* This will invoke suspend on thread behind the scenes
		* @param[in] resetPreviousTime ignored
		*/
		void pause( bool resetPreviousTime ) override;
		/**
		* resume thread
		*/
		void activate() override;
		/**
		* does a join on internal thread
		*/
		void join( unsigned int millis=0xFFFFFFFF );

	private:
		bool	mYieldDone;
		Thread*	mThread;
		static const unsigned short TIME_SLICE;

		/**
		* helper function to initialize object.
		*/
		void initConstruction();
		void threadFinished(  );
		void yieldTask();
		/**
		* used when default constructor is used
		*/
		bool emptyUpdate( ThreadProcess*,bool kill){ return kill;};
	protected:
		/**
		* main behaviour
		*/
		void update(uint64_t milliseconds ) override;
		/**
		* starts the thread
		*/
		void onInit(uint64_t msegs) override;
		/**
		* overridden from Process
		* send terminate signal to thread and check if terminate
		*/
		bool onKill() override;

	};

	template <class F>
	ThreadProcess::ThreadProcess( F functor ) :
		mThread( 0 )
	{
		setPeriod( TIME_SLICE );
		setProcessFunction( functor );
	}
	template <class F> void ThreadProcess::setProcessFunction( F functor )
	{
		//@todo I should reuse existing thread, but it needs some tricks
		//and i need to study in deep
		if ( mThread )
		{
			mThread->finish();
			mThread->join();
			delete mThread;
		}
		mThread = GenericThread::createGenericThread( 
			mpl::addParam_prev<bool,Thread*,bool,void>
			(
				link1st<false,bool,bool>(functor,this,Int2Type<2>() )
			)
			,false,false 
		);
		
		initConstruction();
		//pause( false ); ???//because threads are created as suspended
	}
	Thread* ThreadProcess::getThread() const
	{
		return mThread;
	}

}
