#pragma once

#include <core/Callback.h>
#include <core/ProcessScheduler.h>
#include <core/GenericProcess.h>
#include <core/CallbackSubscriptor.h>
using core::CallbackSubscriptor;

#include <map>
using std::map;

using core::ProcessScheduler;
using core::GenericProcess;

#include <mpl/ParamAdder.h>
using mpl::addParam;
#include <mpl/LinkArgs.h>
using mpl::linkFunctor;
#include <mpl/ReturnAdaptor.h>
using mpl::returnAdaptor;
#include <mpl/MemberEncapsulate.h>
using mpl::makeMemberEncapsulate;
#include <mpl/Functor.h>
using mpl::chain;
#include <mpl/AsPtr.h>
using mpl::asPtr;

#include <core/ThreadDefs.h>

#include <core/Future.h>
using core::Future;
#include <functional>
#include <cassert>

#define RUNNABLE_TASK_ALIGNMENT 8





namespace core 
{
	//some macros to create task functors easily from bool f() functors
	//note: be carefull with f forma, because if it has soma "," inside, maybe you
	// need to use "coma" (defined somewhere in mpl) or use a extra parenthesis
	// for example, if you want to do RUNNABLE_CREATETASK( link1st<false,void>( xxx ) ) this doesn't compile
	//so you need to do RUNNABLE_CREATETASK( (link1st<false,void>( xxx )) ) or RUNNABLE_CREATETASK( link1st<false coma void>( xxx ) )
	//to show de preprocessor that the ',' is not a parameter separator sign
#define RUNNABLE_CREATETASK( f ) \
	addParam< bool,::core::EGenericProcessState,uint64_t,Process*,void >( \
	addParam< bool,Process*,uint64_t,void > \
	(addParam< bool,uint64_t,void >( f ) ) )
//macro to simplify task creation from lambda expression
#define RUNNABLE_CREATELAMBDA_TASK( lambda ) std::function<bool(uint64_t, Process*, ::core::EGenericProcessState)>(lambda)
//useful macro to declare task parameters
#define RUNNABLE_TASK_PARAMS uint64_t t,Process* p,::core::EGenericProcessState s

	template <class TRet, class F> struct ExecuteTask; //predeclaration
	/**
	* @class Runnable
	* @brief A class representing a "running" task, with added functionality to post events requesting
	* execution of external code within it.
	*
	* Any external thread can request any other runnable the execution of any piece
	* of code through Runnable::post(...) methods. 
	*	
	* The execution requests are internaly stored in a ProcessScheduler,and processed anytime
	* after the call is made.
	*
	* Once the request is made, the caller caller is given a "task identifier" that will
	* allow future queries about the task status, through Runnable::waitFor(...) and 
	* Runnable::checkFor(...) methods,
	*
	* The first method simply blocks the calling thread until the requested task
	* has been completed, or a timeout is reached. The second method just checks
	* for task completion and returns inmediately.
	*
	* For this scheme to work, subclasses must ensure that Runnable::processTasks(...) method
	* is called often enough to satisfy the external execution requirements. Otherwise, an
	* IllegalStateException maybe raised to calling threads when a new code execution is
	* made and there is not enough room for the request.
	*/
	class DABAL_API Runnable
	{
		DABAL_CORE_OBJECT_TYPEINFO_ROOT;
	private:
		struct RunnableInfo
		{
			Runnable* current = nullptr;
		};
	public:			
		static const unsigned int DEFAULT_POOL_SIZE = 512;
		enum ETaskPriority {
			HIGH_PRIORITY_TASK = ProcessScheduler::HIGH, 
			NORMAL_PRIORITY_TASK = ProcessScheduler::NORMAL, 
			LOW_PRIORITY_TASK = ProcessScheduler::LOW} ;
		
		//error codes for Future::ErrorInfo when execute a task (see Runnable::execute)
		static const int ERRORCODE_UNKNOWN_EXCEPTION = 1; //when execute detectes exception but is unknown
		static const int ERRORCODE_EXCEPTION = 2; //known exception. ErrorInfo in Future will contain the cloned exception
		//! todo esto no me gusta un pijo
		//!ErrorInfo specialization for execute
		//struct ExecuteErrorInfo : public Future_Base::ErrorInfo
		//{
		//	Exception* exc; //captured exception, cloned  @todo ver si puedo resolver esto
		//	bool isPointer; //if original captured exception was as poiner or object
		//	~ExecuteErrorInfo()
		//	{
		//		if ( !isPointer )
		//		{
		//			delete exc; 
		//		}
		//	}
		//};

		inline ::core::ThreadId getOwnerThreadId() const { assert(mOwnerThread != 0); return mOwnerThread; }
		/**
		* Manually set the owner thread ID.
		* Usually, the thread ID is automatically set when invoking ::run, but
		* this allows the caller to set it up-front, just in case it's needed.
		* @param tid the owner thread to be set.
		*/
		inline void setOwnerThreadId(::core::ThreadId tid) {mOwnerThread=tid;}
		static Runnable* getCurrentRunnable();
	protected:
	private:
		static RunnableInfo* _getCurrentRunnableInfo();
		class DABAL_API RunnableTask final: public GenericProcess
		{
		public:
		
			void* operator new( size_t s,Runnable* owner );
			void operator delete( void* ptr, Runnable* );
			void operator delete( void* ptr );			
			RunnableTask(){}
		private:
		};

		friend class RunnableTask;
		RunnableInfo* mCurrentInfo;
		ProcessScheduler	mTasks;
		unsigned int		mMaxTaskSize;

		struct RTMemPool; //predeclaration
		struct RTMemBlock
		{
			enum class EMemState:uint8_t { FREE = 0,USED = 1 } ;							
			EMemState	memState = EMemState::FREE;  
			alignas(RUNNABLE_TASK_ALIGNMENT) char task[ sizeof( RunnableTask ) ] ;
			//RunnableTask task;
			RTMemPool*	owner;
		};
		typedef list<RTMemPool> MemZoneList;
		struct RTMemPool
		{
			RTMemPool():pool(0),count(0){}
			RTMemBlock*	pool; //array to memory blocks
			Runnable* owner;
			size_t count;
			MemZoneList::iterator iterator;
		};
		MemZoneList mRTZone;
		//! helper function. Create new pool and append it at front
		RTMemPool* _addNewPool();
		//! helper function to remove pool. Internally at least one pool remains, so maybe pool is not removed
		void _removePool( RTMemPool* );

		//RTMemBlock*		mRTMemPool;
		CriticalSection	mMemPoolCS;
		::core::ThreadId	mOwnerThread;//thread executing Runnable
		//typedef Callback<void,Runnable*>	TFinishEvent;
		CallbackSubscriptor<::core::NoMultithreadPolicy, Runnable*> mFinishEvents;
		//list< TFinishEvent* >	mFinishEvents;

		void executeFinishEvents();

		/**
		* helper function for checkCondition
		*/
		template <class Condition>
		bool _checkConditionHelper(  Condition& cond, Future<void>& result )
		{
			if ( cond() )
			{
				result.setValue();
				return true;
			}else
			{
				return false;
			}
		}
	protected:
		/**
		* Performs a controlled loop over the internal queue, excuting
		* pending tasks.
		*/
		void processTasks();
		/**
		* Implements the runnable behaviour. To be overridden by concrete subclasses.
		*/

		virtual unsigned int onRun() = 0;
		virtual unsigned int onPostTask(std::shared_ptr<Process> process,ETaskPriority priority);

		/**
		* set callback to call when a task is added
		*
		* Only one event, so you should get previous event to call chained (and free it)
		* @param[in] functor. With signature void f(Runnable*,Process*)
		* @warning not thread safe. Now it's only protected to allow only to children
		*/
		//template <class F>
		//void setTaskAddedEvent( F functor );
		//inline TTaskAddedEvent* getTaskAddedEvent() const;
	public:




		/**
		* Posts new execution request. Adds the request to the queue and
		* generates a new internal task id.
		* @param[in] process structure containing execution data.
		* @return an integer being the internal task id just created
		* @internally, it calls protected onPostTask, so children can override default behaviour
		*/
		inline unsigned int postTask(std::shared_ptr<Process> process,ETaskPriority priority);

		/**
		* Creates a new runnable object.
		* @param maxTaskSize the maximum pending tasks allowed
		*/
		Runnable(unsigned int maxTaskSize=DEFAULT_POOL_SIZE);
		virtual ~Runnable();

		/**
		* performs some initializations and call onRun
		*/
		unsigned int run();
		/**
		* sends Runnable finish signal
		* you must check for finished() to make sure
		*/ 
		virtual void finish() = 0;
		virtual bool finished() = 0;

		
		/**
		* Posts a new execution request over a functor
		* The execution is NOT guaranteed to be taken into account inmediatly.
		* If caller needs to know whether the code has actualy been run, he must
		* call Runnable::checkFor(...) or Runnable::waitFor(...).
		* @param[in] task_proc the functor to be executed. It has signature: bool (unsigned int msecs, Process*,::core::EGenericProcessState)
		* @param[in] priority process priority
		* @param[in] period Milliseconds
		* @param[in] startTime milliseconds to begin task
		* @param[in] extraInfo Passed to internal created process on setExtraInfo. @todo temporal hasta que haya un extradata en foundation. Problemas con compatibilidad judaica
		*		where first parameter is schedulre time (in milliseconds) and second is envelope Process.
		*		This function must return true if is finished
		* @return the process created for this task
		*/

		template <class F>
		std::shared_ptr<Process> post(
			F&& task_proc,
			bool autoKill = false,
			ETaskPriority priority = NORMAL_PRIORITY_TASK,
			unsigned int period = 0,unsigned int startTime = 0,void* extraInfo = NULL);
		/**
		* executes a function in a context of the Runnable.
		* If this Runnable is in the same thread than caller then, depending on forcepost parameter, the functor
		* will be executed directly (so Future<TRet> will be always available at return) or posted (so caller will
		* need to wait on this Future or whatever other mechanism)
		* @param[in] function Functor with signature TRet f() that will be executed in this Runnable
		* @param[in] forcePost if true, then function will be posted in spite of calling thread is same as Runnable thread
		* @param[in] delay milliseconds to be delayed before execution
		* @param[in] extraInfo Same as in #post
		*/
		template <class TRet,class F> 
			Future<TRet> execute( F&& function,unsigned int delay = 0, bool forcePost = false, void* extraInfo = NULL );
		//template <class TRet>
		//	Future<TRet> execute(std::function< TRet()>&& function, unsigned int delay = 0, bool forcePost = false, void* extrainfo = NULL );

		/**
		* evaluate condition on this Runnable
		* @param[in] cond Condition object. Must have bool operator()()
		* @param[in] checkPeriod Internally it post a task, so this is the task period. Default is 0
		* @return a future which will be triggered when condition is true
		*/
		template <class Condition> 
		Future<void> checkCondition( Condition cond,unsigned int checkPeriod = 0);
		
		/**
		* trigger the given callback when given future is ready (valid or error)
		* @remarks internally it post a task in this Runnable and this task becomes sleep.It will
		* be awaked when future is ready. Some things to consider:
		*		- if the future has a FUTURE_RECEIVED_KILL_SIGNAL because was kill, no callback is called (to avoid when task are being killed and maybe callback is not more valid)
		*		- if trigger is canceled through FutureTriggerInfo, callback is not called
		*/
		class FutureTriggerInfo
		{
		public:
			FutureTriggerInfo():mCancel(false){}
			inline void cancel(){ mCancel = true;}
			inline bool getCancel() const{ return mCancel;}
		private:
			bool mCancel;
		};
		template <class F>
		/**
		* @param[in] future The future to wait for
		* @param[in[ functor Callback called when triggered with signature <void,const Future_Base&>
		* @param[in] autoKill If wait is canceled when kill signal is received
		* @param extraInfo same as in #post
		* @return reference to a FutureTriggerInfo (don't delete it)
		*/
		FutureTriggerInfo* triggerOnDone( const ::core::Future_Base& future, F&& functor, bool autoKill = true, void* extraInfo = NULL);
		/**
		* @param[in] future The future to wait for
		* @param[in] f an standard function to be called when triggered with signature <void,const Future_Base&>
		* @param[in] autoKill If wait is canceled when kill signal is received
		* @param extraInfo same as in #post
		* @return reference to a FutureTriggerInfo (don't delete it)
		*/
		FutureTriggerInfo* triggerOnDone( const ::core::Future_Base& future, std::function<void(const ::core::Future_Base&)>&& f, bool autoKill = true, void* extraInfo = NULL);
        
		/**
		* subscribe to finish event. This event will be executed when Runnable finish,
		* so, when onRun returns;
		*/
		template <class F>
		void subscribeFinishEvent( F&& functor);
		
		/**
		* Blocks the caller until the given task has been executed by this object.
		* This function must be called form a different Thread than in which this Runnable is executed.
		* @param taskId the task to wait for
		* @param millis timeout (milliseconds)
		* @return `true+  if the task is complished before the timeout expires. `false` otherwise
		* @remarks if task was not completed this is taken off scheduler
		* @todo esta funcion no es muy coherente aqu�, ya que no se puede saber si est� en otro thread
		* @deprecated Use futures. This function might disappear in future versions
		*/
#ifndef _WINDOWS
		///@cond
		__attribute__((deprecated))
		///endcond
#endif
		bool waitFor(const unsigned int taskId, const unsigned int millis);


		/**
		* Checks for a given task to be acomplished
		* @param[in] taskId the task to check
		* @return true if the task has already been executed. false otherwise
		*/
		inline bool checkFor(const unsigned int taskId);
		inline const ProcessScheduler& getTasksScheduler() const;
		inline ProcessScheduler& getTasksScheduler() ;

		/**
		* set timer to be used by internal scheduler
		* If no Timer is given, a new Timer is created first time
		*/
		void setTimer( std::shared_ptr<Timer> timer );
		inline const std::shared_ptr<Timer> getTimer( ) const;
		inline std::shared_ptr<Timer> getTimer( );
		inline unsigned int getPendingTaskCount() const 
		{
			return (unsigned int)getTasksScheduler().getProcessCount();
		}
		/*
		* get number of processes running (not sleeped or paused)
		*/
	//	intentar flexibilizarlo para cuando considere los wait
		inline unsigned int getActiveTaskCount() const 
		{
			return (unsigned int)getTasksScheduler().getActiveProcessCount();
		}
	private:
		/**
		* helper for triggerOnDone
		*/
		void _triggerOnDone( const ::core::Future_Base& future, Callback<void,const Future_Base&>*,FutureTriggerInfo* info );
		static void _sleep( unsigned int);

	};

	
	template <class F>
	std::shared_ptr<Process> Runnable::post(
		F&& task_proc,
		bool autoKill,
		ETaskPriority priority ,
		unsigned int period,unsigned int startTime,void* extraInfo  )
	{
		::std::shared_ptr<RunnableTask> p (new (this)RunnableTask());
		//GenericProcess* p = new GenericProcess();
		p->setProcessCallback( ::std::forward<F>(task_proc) );
		p->setPeriod( period );
		p->setStartTime( startTime );
		p->setExtraInfo( extraInfo );
		p->setAutoKill( autoKill );
		postTask(p,priority);
		return p;
	}


	
	bool Runnable::checkFor(const unsigned int taskId) {
		bool taskCompleted;
		taskCompleted = mTasks.checkFor(taskId);
		return taskCompleted;
	}
	const ProcessScheduler& Runnable::getTasksScheduler() const
	{
		return mTasks;
	}
	ProcessScheduler& Runnable::getTasksScheduler() 
	{
		return mTasks;
	}
	const std::shared_ptr<Timer> Runnable::getTimer( ) const
	{
		return mTasks.getTimer();
	}
	std::shared_ptr<Timer> Runnable::getTimer( )
	{
		return mTasks.getTimer();
	}
	template <class F>
	void Runnable::subscribeFinishEvent( F&& functor)
	{
		//mFinishEvents.push_back( new TFinishEvent( functor, ::core::use_functor ) );
		mFinishEvents.subscribeCallback(std::forward<F>(functor));
	}
	template <class TRet,class F> 
	Future<TRet> Runnable::execute( F&& function,unsigned int delay,bool forcePost, void* extraInfo)
	{
		assert(mOwnerThread != 0);
		Future<TRet> future;
		if ( forcePost || mOwnerThread != ::core::getCurrentThreadId())
		{
			//@todo no usada la version con linkfunctor porque tendr�a que hacer yo el std::move cuando sea y dem�s, y resulta m�s f�cil verlo (y sobretodo depurarlo) sin ello
			//y que el propio ExecuteTask lo resuelva en el constructor
			//post(
			//	RUNNABLE_CREATETASK
			//	(
			//		returnAdaptor<void>
			//		(				
			//			linkFunctor<void,TYPELIST()>( ExecuteTask<TRet,typename ::std::decay<F>::type>(),function,future )
			//			,true
			//		)
			//	),false/*@todo esto tengo que poder configurarlo*/
			//	,NORMAL_PRIORITY_TASK
			//	,0,delay, extraInfo

			//);
			post(
				RUNNABLE_CREATETASK
				(
					returnAdaptor<void>
					(
						linkFunctor<void, TYPELIST()>(ExecuteTask<TRet, typename ::std::decay<F>::type>(::std::forward<F>(function)), future)
						, true
						)
				), false/*@todo esto tengo que poder configurarlo*/
				, NORMAL_PRIORITY_TASK
				, 0, delay, extraInfo

			);
	
		}else
		{
			_sleep( delay ); //@todo �apa para evitar incluir Thread.h
			//same calling thread that this thread, execute directly
			ExecuteTask<TRet, typename ::std::decay<F>::type>(::std::forward<F>(function))( future);
		}

		return future;
	}
	/*template <class TRet>
	Future<TRet> Runnable::execute(std::function< TRet()>&& function, unsigned int delay, bool forcePost, void* extraInfo)
	{
		assert(mOwnerThread != 0);
		Future<TRet> future;
		if (forcePost || mOwnerThread != ::core::getCurrentThreadId())
		{
			post(
				std::function< bool( unsigned int, Process*, ::core::EGenericProcessState) >(
				RUNNABLE_CREATETASK
				(
					returnAdaptor<void>
					(
						linkFunctor<void, TYPELIST()>(ExecuteTask<TRet, typename std::function< TRet()> >(), function, future)
						, true
						)
				)), false
				, NORMAL_PRIORITY_TASK
				, 0, delay, extraInfo

			);

		}
		else
		{
			_sleep(delay); //@todo �apa para evitar incluir Thread.h
						   //same calling thread that this thread, execute directly
			ExecuteTask<TRet, std::function< TRet()> > rt;
			rt(function, future);
		}
		return future;
	}*/
/*	template <class Condition> bool _ch(Condition& cond,Future<void>&)
	{
		return true;
	}*/
	template <class Condition> 
	Future<void> Runnable::checkCondition( Condition cond,unsigned int checkPeriod )
	{
		assert(mOwnerThread != 0);
		Future<void> result;
		if ( (mOwnerThread != ::core::getCurrentThreadId()) || !_checkConditionHelper(cond,result))
		{
			//post task to check condition
			
			 post(
				RUNNABLE_CREATETASK((
					linkFunctor<bool,TYPELIST()>(makeMemberEncapsulate( &Runnable::_checkConditionHelper<Condition>,this ),
												 cond,result)
				))
				
			);
			
			

		}
		
		return result;
	}
	unsigned int Runnable::postTask(std::shared_ptr<Process> process,Runnable::ETaskPriority priority)
	{
		return onPostTask( process, priority );
	}
	template <class F>
	Runnable::FutureTriggerInfo* Runnable::triggerOnDone( const ::core::Future_Base& future, F&& functor,bool autoKill, void* extraInfo)
	{
		if ( !future.getValid() )
		{
			FutureTriggerInfo* info = new FutureTriggerInfo;
			typedef Callback<void,const ::core::Future_Base&> TCallback;
			TCallback* cb = new TCallback( functor, ::core::use_functor );
			post( 
				RUNNABLE_CREATETASK(
					returnAdaptor<void>
					(
						linkFunctor<void,TYPELIST()>( makeMemberEncapsulate( &Runnable::_triggerOnDone, this ),future,cb,info)
						,true
					)
				),autoKill,::core::Runnable::NORMAL_PRIORITY_TASK,0,0,extraInfo
			);
			return info;
		}else
		{
			functor( future );
			return NULL;
		}
	}

	///@cond HIDDEN_SYMBOLS
	//helper class por request task to Runnable
	template <class TRet, class F> struct ExecuteTask
	{
		F mFunction;
		ExecuteTask(F&&f):mFunction(std::move(f))
		{			
		}
		ExecuteTask(const F& f):mFunction(f)
		{
		}
		void operator()( /*F& function,*/ Future<TRet> f )
		{
			try
			{
				//f.setValue( function() );
				f.setValue(mFunction());
			}
			//check chances of Exception
			catch( std::exception& e )
			{
				/*Runnable::ExecuteErrorInfo* ei = new Runnable::ExecuteErrorInfo;
				ei->error = Runnable::ERRORCODE_EXCEPTION;
				ei->exc = e.clone();
				ei->isPointer = false;*/
				auto ei = new Future_Base::ErrorInfo;
				ei->error = Runnable::ERRORCODE_EXCEPTION;
				ei->errorMsg = e.what();
				f.setError( ei );	
			}
			catch(...)
			{
				/*Future_Base::ErrorInfo* ei = new Future_Base::ErrorInfo; 
				ei->error = Runnable::ERRORCODE_UNKNOWN_EXCEPTION;
				ei->errorMsg = "Unknown exception";*/
				auto ei = new Future_Base::ErrorInfo;
				ei->error = Runnable::ERRORCODE_UNKNOWN_EXCEPTION;
				ei->errorMsg = "Unknown exception";
				f.setError( ei );	
			}

		}
		bool operator ==( const ExecuteTask& ) const{ return true;} //for compliance
	};
	//specialization for void TRet
	template <class F> struct ExecuteTask<void,F>
	{
		F mFunction;
		ExecuteTask(F&&f) :mFunction(std::move(f))
		{
		}
		ExecuteTask(const F& f) :mFunction(f)
		{
		}
		void operator()( Future<void> f )
		{
			try
			{
				mFunction();
				f.setValue( );
			}
			//check chances of Exception
			catch( std::exception& e )
			{
				auto ei = new Future_Base::ErrorInfo;
				ei->error = Runnable::ERRORCODE_EXCEPTION;
				ei->errorMsg = e.what();
				f.setError( ei );	
			}
			catch(...)
			{
				auto ei = new Future_Base::ErrorInfo;
				ei->error = Runnable::ERRORCODE_UNKNOWN_EXCEPTION;
				ei->errorMsg = "Unknown exception";
				f.setError( ei );	
			}
		}
		bool operator ==( const ExecuteTask& )const { return true;} //for compliance
	};
	///@endcond

}
