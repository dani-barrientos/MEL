///////////////////////////////////////////////////////////
//  ProcessScheduler.h
//  Implementation of the Class ProcessScheduler
//  Created on:      29-mar-2005 10:00:12
///////////////////////////////////////////////////////////

#pragma once

#include <tasking/Process.h>

#include <set>
using std::set;

#include <list>
using std::list;
using tasking::Process;

using std::pair;


#include <core/CriticalSection.h>
using core::CriticalSection;

#include <core/Timer.h>
using core::Timer;
#include <core/CallbackSubscriptor.h>
using core::CallbackSubscriptor;
#include <mpl/Int2Type.h>
using ::mpl::Int2Type;
#include <utility>
#include <atomic>

// temas que quiero:
//  - quitar esas prioridades. meter un objeto de propiedades en el post donde se indiquen cosas, si es que las quiero meter
//   - ¿poder insertar al principio o al final?->¿de verdad tiene sentido? al final es un bucle infinito donde no importa. Lo que si puede
//   importar es insertar antes o después que otra¿¿??


namespace tasking
{
	typedef std::pair<mpl::Int2Type<0>,CallbackSubscriptor< ::core::NoMultithreadPolicy, std::shared_ptr<Process>>> SleepSubscriptor;
	typedef std::pair<mpl::Int2Type<1>,CallbackSubscriptor< ::core::MultithreadPolicy, std::shared_ptr<Process>>> WakeSubscriptor;
	typedef std::pair<mpl::Int2Type<2>,CallbackSubscriptor< ::core::NoMultithreadPolicy, std::shared_ptr<Process>>> EvictSubscriptor;
    /**
    * Process manager. It can be seen as a task scheduler
    * @version 1.0
    * @remarks as is explained in Process, temporization is used with a Timer but getting only the 32 bit low part of the
    * 64 bits time returned, for efficiency reasons.
    */
	class DABAL_API ProcessScheduler : private  SleepSubscriptor //TODO que poco me gusta esta herencia, incrementa el tama�o de los Process y quisiera que fuesen m�s ligeros
		,private WakeSubscriptor
		,private EvictSubscriptor
	{
		typedef unsigned int ThreadID;
		friend class Process;
	public:
		typedef list< std::pair<std::shared_ptr<Process>,unsigned int> > TProcessList; //pairs of processes and starttime
		ProcessScheduler();		
		~ProcessScheduler(void);

		/**
		* =============================================================================
		*
		* @param process insert new process in scheduler
		* @param starttime msecs to wait to start it
		* @warning 	if process is already inserted behaviour is unpredictable
		*/
		void insertProcess( std::shared_ptr<Process> process,unsigned int startTime = 0);


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
		* @todo no me gusta, hay que generalizarlo mejor. Como obtener los procesos que cunplen algo y actuar sobre ellos
		*/
		template <class T>
		void pauseProcesses(T& predicate);

		
		/**
		* =============================================================================
		*/
		void activateProcesses();
		/**
		* send processes kill signal. It doesn�t kill inmediately, it's done at execution cycle begin, so
		* you need to continue processing until no more process are pending (getProcessCount() == 0)
		* @param[in] deferred If true, then kill will be posted as a new process. If False, it's done inmediately.
		* First type is when done into Scheduler context and second form is when done from another thread
		@todo no me gusta
		*/
		void killProcesses( bool deferred );

		/**
		* Checks for a given task to be finished
		* @param[in] taskId the task to check
		* @return true if the task has already been executed. false otherwise
		* @warning  this function is not thread-safe. If you need it you
		*  must block executeprocess() and checkFor() with a critical section
		*/
		//inline bool checkFor(unsigned int taskId);

		inline TProcessList& getProcesses();
		inline TProcessList& getPendingProcesses();
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
		* subscribe to process sleep event
		*/
		template <class F> int susbcribeSleepEvent(F&& f)
		{
			return SleepSubscriptor::second.subscribeCallback(std::forward<F>(f));
		}
		template <class F> void unsusbcribeSleepEvent(F&& f)
		{
			SleepSubscriptor::second.unsubscribeCallback(std::forward<F>(f));
		}
		void unsusbcribeSleepEvent(int id)
		{
			SleepSubscriptor::second.unsubscribeCallback(id);
		}
		template <class F> int susbcribeWakeEvent(F&& f)
		{
			return WakeSubscriptor::second.subscribeCallback(std::forward<F>(f));
		}
		template <class F> void unsusbcribeWakeEvent(F&& f)
		{
			WakeSubscriptor::second.unsubscribeCallback(std::forward<F>(f));
		}
		template <class F> int subscribeProcessEvicted(F&& f)
		{
			return EvictSubscriptor::second.subscribeCallback(std::forward<F>(f));
		}
		template <class F> void unsubscribeProcessEvicted(F&& f)
		{
			EvictSubscriptor::second.unsubscribeCallback(std::forward<F>(f));
		}
		void unsubscribeProcessEvicted(int id)
		{
			EvictSubscriptor::second.unsubscribeCallback(id);
		}
	private:
		struct ProcessInfo  //for TLS
		{
			std::shared_ptr<Process> current;
		};
		ProcessInfo*	mProcessInfo;
		TProcessList mProcessList;
		//new processes to insert next time
		 TProcessList  mNewProcesses;

		mutable CriticalSection	mCS;		
		std::shared_ptr<Timer>			mTimer;
		//unsigned int			mProcessCount;
		std::atomic<unsigned int> mProcessCount;
		volatile int32_t		mInactiveProcessCount;
		std::shared_ptr<Process>	mPreviousProcess;
		bool					mKillingProcess; //flag to mark when ther is a kill task pending

		/**
		* helper function
		*/
		void _executeProcesses(uint64_t time,TProcessList& processes) OPTIMIZE_FLAGS;

		void _killTasks();
		void _triggerSleepEvents(std::shared_ptr<Process> p);
		void _triggerWakeEvents(std::shared_ptr<Process> p);
		static ProcessInfo* _getCurrentProcessInfo();
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
	};
	/*std::shared_ptr<Process> ProcessScheduler::getProcessForId( unsigned int taskId ) const
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
*/

	template <class T>
	void ProcessScheduler::pauseProcesses(T& predicate)
	{
		for(auto i = mProcessList.begin() ; i != mProcessList.end(); ++i)
		{
			if ( predicate( *i ) )
			{
				(*i).first->pause();
			}
		}
	}
	ProcessScheduler::TProcessList& ProcessScheduler::getProcesses() 
	{
		return mProcessList;
	}
	ProcessScheduler::TProcessList& ProcessScheduler::getPendingProcesses()
	{
		return mNewProcesses;
	}
	
	template <class Predicado> bool ProcessScheduler::forEach( Predicado pred)
	{
		TProcessList::const_iterator i;
		const TProcessList *processArray[2];
		const TProcessList* aux;
		processArray[0] = &getProcesses();
		processArray[1] = &mNewProcesses;
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
		//accediendo al size de cada lista puede dar una condici�n de carrera
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
			if ( function( i->first ) )
				processList.push_back( i->first );
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

