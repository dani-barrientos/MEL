///////////////////////////////////////////////////////////
//  ProcessScheduler.h
//  Implementation of the Class ProcessScheduler
//  Created on:      29-mar-2005 10:00:12
///////////////////////////////////////////////////////////

#pragma once

#include <core/Process.h>

#include <set>
using std::set;

#include <list>
using std::list;
using core::Process;

#include <unordered_map>
using std::unordered_map;
using std::pair;


#include <core/CriticalSection.h>
using core::CriticalSection;

#include <core/Timer.h>
using core::Timer;
#include <core/CallbackSubscriptor.h>
using core::CallbackSubscriptor;
#include <mpl/Int2Type.h>
using ::mpl::Int2Type;

namespace core
{
	typedef CallbackSubscriptor<mpl::Int2Type<0>, true, bool, std::shared_ptr<Process>> SleepSubscriptor;
	typedef CallbackSubscriptor<mpl::Int2Type<1>, true, bool, std::shared_ptr<Process>> WakeSubscriptor;
	typedef CallbackSubscriptor<mpl::Int2Type<2>, true, bool, std::shared_ptr<Process>> EvictSubscriptor;
    /**
    * Process manager. It can be seen as a task scheduler
    * @version 1.0
    * @remarks as is explained in Process, temporization is used with a Timer but getting only the 32 bit low part of the
    * 64 bits time returned, for efficiency reasons.
    */
	class FOUNDATION_API ProcessScheduler : private  SleepSubscriptor //TODO que poco me gusta esta herencia, incrementa el tamaño de los Process y quisiera que fuesen más ligeros
		,private WakeSubscriptor
		,private EvictSubscriptor
	{
		typedef unsigned int ThreadID;
	public:
		typedef list< std::shared_ptr<Process> > TProcessList;
		//!todo lo de las prioridades muy primitivo, por eso está aquí fuleramente
		//!en realidad ahora sólo significan si el proceso se ejecuta el primero de la lista, en el medio o al final
		enum EProcessPriority
		{
			HIGH = 0,NORMAL,LOW
		};
		/**
		* @todo gestionarPrioridades
		*/
		ProcessScheduler();
		/**
		*/
		virtual ~ProcessScheduler(void);
		void ini();


		/**
		* =============================================================================
		*
		* @param process insert new process in scheduler
		* @param priority process priority
		* @warning 	if process is already inserted behaviour is unpredictable
		* @return taks identifier.
		*/
		unsigned int insertProcess( std::shared_ptr<Process> process,EProcessPriority priority);


		/**
		*  gets how many process in scheduler
		*/
		inline unsigned int getProcessCount() const;
		/*
		* get number of processes running (not sleeped)
		*/
		inline unsigned int getActiveProcessCount() const;
		/**
		* =============================================================================
		*/
		void executeProcesses();
		/**
		* ! pausa todos los procesos
		*/
		void pauseProcesses();
		/**
		* Pauses proceses matching the given predicate
		*
		* @param predicate predicate
		*/
		template <class T>
		void pauseProcesses(T& predicate);

		// REHACER CORRECTAMENTE PARA CONJUNTOS DE Type
		//void pauseProcesses(const set<int>& tipos);
		/**
		* =============================================================================
		*/
		void activateProcesses();
		/**
		* send processes kill signal. It doesn´t kill inmediately, it's done at execution cycle begin, so
		* you need to continue processing until no more process are pending (getProcessCount() == 0)
		* @param[in] deferred If true, then kill will be posted as a new process. If False, it's done inmediately.
		* First type is when done into Scheduler context and second form is when done from another thread
		*/
		void killProcesses( bool deferred );

		/**
		* Checks for a given task to be finished
		* @param[in] taskId the task to check
		* @return true if the task has already been executed. false otherwise
		* @warning  this function is not thread-safe. If you need it you
		*  must block executeprocess() and checkFor() with a critical section
		*/
		inline bool checkFor(unsigned int taskId);

		/**
		* return Process pointer for given task id. YOu can't take ownership
		*/
		inline std::shared_ptr<Process> getProcessForId( unsigned int taskId ) const;

		inline const TProcessList& getInitialProcesses() const;
		inline const TProcessList& getFinalProcesses() const;
		/**
		* @todo tengo que revisar estas funciones que no me convencen un pijo así
		*/
		inline TProcessList& getProcesses();
		/**
		* get new processigs to be scheduled (recently inserted)
		*/
		inline TProcessList& getPendingProcesses();
		inline TProcessList& getPendingInitialProcesses();
		inline TProcessList& getPendingFinalProcesses();
		/**
		* do pred for each process in scheduler.
		* @param[in] pred function of type: bool f(Process*)
		* @return true if pred return true for some process
		*/
		template <class Predicado> bool forEach( Predicado pred);
		/**
		* coge los procesos que cumplem el predicado indicado (function)
		*/
		template <class T> void getProcesses( T function, list<std::shared_ptr<Process>>& processList );


		/**
		* remove all processes without taking care
		* @remarks for use with caution
		*/
		void destroyAllProcesses();

		/**
		* set Timer to use.
		*/
		void setTimer(std::shared_ptr<Timer> timer );
		/**
		* get used Timer
		*/
		inline const std::shared_ptr<Timer> getTimer() const;
		inline std::shared_ptr<Timer> getTimer() ;

        /**
        * get current executing process in current thread
        */
		static std::shared_ptr<Process> getCurrentProcess();
		/**
		* internal method intended to be used by Process to notify was awakened
		* @todo maybe this will be deprecated in favor of another mechanism (CallbackSubscriptor, etc)
		*/
		void processAwakened(std::shared_ptr<Process>);
		/**
		* internal method intended to be used by Process to notify was asleep
		* @todo maybe this will be deprecated in favor of another mechanism (CallbackSubscriptor, etc)
		*/
		void processAsleep(std::shared_ptr<Process>);
		/**
		* subscribe to process sleep event
		*/
		template <class F> int susbcribeSleepEvent(F&& f)
		{
			return SleepSubscriptor::subscribeCallback(std::forward<F>(f));
		}
		template <class F> void unsusbcribeSleepEvent(F&& f)
		{
			SleepSubscriptor::unsubscribeCallback(std::forward<F>(f));
		}
		void unsusbcribeSleepEvent(int id)
		{
			SleepSubscriptor::unsubscribeCallback(id);
		}
		template <class F> int susbcribeWakeEvent(F&& f)
		{
			return WakeSubscriptor::subscribeCallback(std::forward<F>(f));
		}
		template <class F> void unsusbcribeWakeEvent(F&& f)
		{
			WakeSubscriptor::unsubscribeCallback(std::forward<F>(f));
		}
		template <class F> int subscribeProcessEvicted(F&& f)
		{
			return EvictSubscriptor::subscribeCallback(std::forward<F>(f));
		}
		template <class F> void unsubscribeProcessEvicted(F&& f)
		{
			EvictSubscriptor::unsubscribeCallback(std::forward<F>(f));
		}
		void unsubscribeProcessEvicted(int id)
		{
			EvictSubscriptor::unsubscribeCallback(id);
		}
	private:
		struct ProcessInfo  //for TLS
		{
			std::shared_ptr<Process> current;
		};
		ProcessInfo*	mProcessInfo;
		TProcessList mProcessList;
		TProcessList mFinalProcesses;
		TProcessList mInitialProcesses;
		//new processes to insert next time
		TProcessList mNewProcesses;
		TProcessList mNewInitialProcesses;
		TProcessList mNewFinalProcesses;

		mutable CriticalSection	mCS;
		mutable CriticalSection	mPendingIdTasksCS;
		//mutable CriticalSection	mExecutionCS;

		std::shared_ptr<Timer>			mTimer;
		unsigned int			mRequestedTaskCount;
		unsigned int			mProcessCount;
		volatile int32_t		mInactiveProcessCount;
		unordered_map<unsigned int, std::shared_ptr<Process>>	mPendingIdTasks;
		//list <std::shared_ptr<Process>>			mNew;

		std::shared_ptr<Process>				mPreviousProcess;
		bool					mKillingProcess; //flag to mark when ther is a kill task pending

		/**
		* helper function
		*/
		void executeProcesses(uint64_t time,TProcessList& processes) OPTIMIZE_FLAGS;

		void killTask();
		void _triggerSleepEvents(std::shared_ptr<Process> p);
		void _triggerWakeEvents(std::shared_ptr<Process> p);
		static ProcessInfo* _getCurrentProcessInfo();
	};
	std::shared_ptr<Process> ProcessScheduler::getProcessForId( unsigned int taskId ) const
	{
		std::shared_ptr<Process> result = 0;
		mPendingIdTasksCS.enter();
		auto pos = mPendingIdTasks.find( taskId );
		if ( pos != mPendingIdTasks.end() )
		{
			result = pos->second;
		}
		mPendingIdTasksCS.leave();
		return result;
	}


	template <class T>
	void ProcessScheduler::pauseProcesses(T& predicate)
	{
		TProcessList::iterator	i;

		for( i = mInitialProcesses.begin(); i != mInitialProcesses.end(); ++i)
		{
			if ( predicate( *i ) )
			{
				(*i)->pause();
			}
		}
		for(i = mProcessList.begin() ; i != mProcessList.end(); ++i)
		{
			if ( predicate( *i ) )
			{
				(*i)->pause();
			}
		}
		for( i = mFinalProcesses.begin(); i != mFinalProcesses.end(); ++i)
		{
			if ( predicate( *i ) )
			{
				(*i)->pause();
			}
		}

	}
	bool ProcessScheduler::checkFor(const unsigned int taskId)
	{
		bool taskCompleted;
		mPendingIdTasksCS.enter();
		taskCompleted = (mPendingIdTasks.find( taskId ) == mPendingIdTasks.end() );
		mPendingIdTasksCS.leave();
		return taskCompleted;
	}
	const ProcessScheduler::TProcessList& ProcessScheduler::getInitialProcesses() const
	{
		return mInitialProcesses;
	}
	const ProcessScheduler::TProcessList& ProcessScheduler::getFinalProcesses() const
	{
		return mFinalProcesses;
	}
	ProcessScheduler::TProcessList& ProcessScheduler::getProcesses() 
	{
		return mProcessList;
	}
	ProcessScheduler::TProcessList& ProcessScheduler::getPendingProcesses()
	{
		return mNewProcesses;
	}
	ProcessScheduler::TProcessList& ProcessScheduler::getPendingInitialProcesses()
	{
		return mNewInitialProcesses;
	}
	ProcessScheduler::TProcessList& ProcessScheduler::getPendingFinalProcesses()
	{
		return mFinalProcesses;
	}
	template <class Predicado> bool ProcessScheduler::forEach( Predicado pred)
	{
		TProcessList::const_iterator i;
		const TProcessList *processArray[6];
		const TProcessList* aux;
		processArray[0] = &getInitialProcesses();
		processArray[1] = &getFinalProcesses();
		processArray[2] = &getProcesses();
		processArray[3] = &mNewProcesses;
		processArray[4] = &mNewInitialProcesses;
		processArray[5] = &mNewFinalProcesses;
		bool res = false;

		int count = 0;
		while( count < 6 )
		{
			aux = processArray[ count ];
			for( i = aux->begin(); i != aux->end(); ++i)
			{
				if (pred( *i ) )
				{
					res = true;
				}
			}
			count++;
		}
		return res;
	}
	unsigned int ProcessScheduler::getProcessCount() const
	{
		//accediendo al size de cada lista puede dar una condición de carrera
		/*return (unsigned int)(mNewProcesses.size() + mNewFinalProcesses.size() + mNewInitialProcesses.size() +
			mProcessList.size() + mFinalProcesses.size() + mInitialProcesses.size());
			*/
		unsigned int result;
		//mCS.enter();
		result = mProcessCount; 
		//mCS.leave();
		return result;
	}
	unsigned int ProcessScheduler::getActiveProcessCount() const
	{
		return getProcessCount()-((unsigned int)mInactiveProcessCount);
	}
	template <class T> void ProcessScheduler::getProcesses( T function, list<std::shared_ptr<Process>>& processList )
	{
		TProcessList::iterator i;
		for( i = mProcessList.begin(); i != mProcessList.end(); ++i )
		{
			if ( function( *i ) )
				processList.push_back( *i );
		}
		for( i = mInitialProcesses.begin(); i != mInitialProcesses.end(); ++i )
		{
			if ( function( *i ) )
				processList.push_back( *i );
		}
		for( i = mFinalProcesses.begin(); i != mFinalProcesses.end(); ++i )
		{
			if ( function( *i ) )
				processList.push_back( *i );
		}

	}
	const std::shared_ptr<Timer> ProcessScheduler::getTimer() const
	{
		return mTimer;
	}
	std::shared_ptr<Timer> ProcessScheduler::getTimer()
	{
		return mTimer;
	}

}

