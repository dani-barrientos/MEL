#pragma once

#include <core/Callback.h>
#include <tasking/ProcessScheduler.h>
#include <tasking/GenericProcess.h>
#include <core/CallbackSubscriptor.h>
using core::CallbackSubscriptor;

#include <map>
using std::map;

using tasking::ProcessScheduler;
using tasking::GenericProcess;

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
using core::Future_Base;
#include <functional>
#include <cassert>
#include <forward_list>
#include <mutex>

#define RUNNABLE_TASK_ALIGNMENT 8

namespace tasking 
{
	//some macros to create task functors easily from bool f() functors
	//note: be carefull with f forma, because if it has soma "," inside, maybe you
	// need to use "coma" (defined somewhere in mpl) or use a extra parenthesis
	// for example, if you want to do RUNNABLE_CREATETASK( link1st<false,void>( xxx ) ) this doesn't compile
	//so you need to do RUNNABLE_CREATETASK( (link1st<false,void>( xxx )) ) or RUNNABLE_CREATETASK( link1st<false coma void>( xxx ) )
	//to show de preprocessor that the ',' is not a parameter separator sign
#define RUNNABLE_CREATETASK( f ) \
	addParam< ::tasking::EGenericProcessResult,Process*,uint64_t,void > \
	(addParam< ::tasking::EGenericProcessResult,uint64_t,void >( f ) ) 
//macro to simplify task creation from lambda expression
#define RUNNABLE_CREATELAMBDA_TASK( lambda ) std::function<::tasking::EGenericProcessResult (uint64_t, Process*)>(lambda)
//useful macro to declare task parameters
#define RUNNABLE_TASK_PARAMS uint64_t t,Process* p
	class Runnable; //predeclaration
	namespace _private
	{
		class DABAL_API RunnableTask final: public GenericProcess
		{
		public:		
			//throws bad_alloc oif no enoguh memory
			static void* operator new( size_t s,Runnable* owner );
			void operator delete(void* ptr, Runnable*) noexcept;
			static void operator delete( void* ptr ) noexcept;			
			RunnableTask(){}
		private:
		};
		//default allocator for new tasks (through post) doing a simple new 	
		template <class T>
		struct Allocator
		{
			static T* allocate(Runnable* _this)
			{
				return new T();
			}
		};
			//special allocator for RunnableTask using internal pool
		template <>
		struct Allocator<RunnableTask>
		{
			static RunnableTask* allocate(Runnable* _this)
			{        
				return new (_this)RunnableTask();
			}
		};
		struct RTMemPool; //predeclaration
		struct RTMemBlock
		{
			enum class EMemState:uint8_t { FREE = 0,USED = 1 } ;							
			EMemState	memState = EMemState::FREE;  
			alignas(RUNNABLE_TASK_ALIGNMENT) char task[ sizeof( ::tasking::_private::RunnableTask ) ] ;			
			//RunnableTask task;
			RTMemPool*	owner;
		};
		struct MemZoneList
		{
			typedef std::forward_list<RTMemPool> ListType;
			public:
				MemZoneList():mSize(0){}
				size_t size(){ return mSize;};
				void push_front(RTMemPool&& pool)
				{
					mList.push_front(std::move(pool));
					++mSize;
				}
				 ListType& getList(){ return mList;}
				void remove(RTMemPool* pool )
				{
					//@todo hacer más eficiente teniendo iterador
					mList.remove_if([pool](const RTMemPool& p)
					{
						auto r = (pool == &p);
						return r;
					});
					--mSize; //todo no guaratee was removed. Since C++20 remove returns number of elements removed
				}
			private:
				size_t mSize;
				ListType mList;
		};
		//typedef std::forward_list<RTMemPool> MemZoneList;
		struct RTMemPool
		{
			RTMemPool():pool(0),count(0){}
			RTMemBlock*	pool; //array to memory blocks
			Runnable* owner;
			size_t count;
			//MemZoneList::iterator iterator; quitar
		};
	}
	///@cond HIDDEN_SYMBOLS
	template <class TRet,class ErrorType, class F> struct ExecuteTask; //predeclaration
	///@endcond
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
		
		//error codes for Future::ErrorInfo when execute a task (see Runnable::execute)
		static const int ERRORCODE_UNKNOWN_EXCEPTION = 1; //when execute detectes exception but is unknown
		static const int ERRORCODE_EXCEPTION = 2; //known exception. ErrorInfo in Future will contain the cloned exception		
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
	public:  //por temas de depuracion

	private:
		static RunnableInfo* _getCurrentRunnableInfo();
		friend class ::tasking::_private:: RunnableTask;
		RunnableInfo* mCurrentInfo;
		ProcessScheduler	mTasks;
		unsigned int		mMaxTaskSize;  //max number of tasks for each pool (the number of pools is dynamic)

		::tasking::_private::MemZoneList mRTZone;
		//! helper function. Create new pool and append it at front
		::tasking::_private::RTMemPool* _addNewPool();
		//! helper function to remove pool. Internally at least one pool remains, so maybe pool is not removed
		void _removePool( ::tasking::_private::RTMemPool* );

		//RTMemBlock*		mRTMemPool;
		CriticalSection	mMemPoolCS;
		//std::mutex mMemPoolCS;
		::core::ThreadId	mOwnerThread;//thread executing Runnable
		//typedef Callback<void,Runnable*>	TFinishEvent;
		CallbackSubscriptor<::core::CSNoMultithreadPolicy, Runnable*> mFinishEvents;
		void executeFinishEvents();
		
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
		virtual void onPostTask(std::shared_ptr<Process> process){};

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
		* @internally, it calls protected onPostTask, so children can add custom behaviour
		*/
		void postTask(std::shared_ptr<Process> process,unsigned int startTime = 0,bool lockScheduler = true);

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
		* By default, a ::core::_private::RunnableTask is created, which is intended to be used with a custom memory manager for performance reasons.wich also can
		* be changed with template parameter AllocatorType. Users can provide their own ProcessType class (which must inherit from Process o, better, GenericProcess -this is not mandatory, but for simplicity-)
		* This way, user can provide its custom Process class holding custom attributes and/or provide its custom memory manager. @see RTMemPool @see ::core::_private::Allocator for interfaz needed to your custom allocator
		* @param[in] task_proc the functor to be executed. It has signature: bool (unsigned int msecs, Process*)
		* @param[in] period Milliseconds
		* @param[in] startTime milliseconds to begin task
		* @return the process created for this task
		*/
		template <class ProcessType = ::tasking::_private::RunnableTask,class AllocatorType = ::tasking::_private::Allocator<ProcessType>, class F>
		std::shared_ptr<Process> post(
			F&& task_proc,
			bool autoKill = false,
			/*ETaskPriority priority = NORMAL_PRIORITY_TASK, @todo aqui meter un struct de opciones con las que crear el proceso*/
			unsigned int period = 0,unsigned int startTime = 0,
			bool lockScheduler = true);
		/**
		 * @brief Convenient function to post no periodic task with signature void f()
		 * @param[in] task_proc functor with signature void f(void)
		 */
		template <class ProcessType = ::tasking::_private::RunnableTask,class AllocatorType = ::tasking::_private::Allocator<ProcessType>, class F>		
		std::shared_ptr<Process> fireAndForget(
			F&& task_proc,
			unsigned int startTime = 0,
			bool lockScheduler = true);
		/**
		* executes a function in a context of the Runnable.
		* If this Runnable is in the same thread than caller then, depending on forcepost parameter, the functor
		* will be executed directly (so Future<TRet> will be always available at return) or posted (so caller will
		* need to wait on this Future or whatever other mechanism)
		* @param[in] function Functor with signature TRet f() that will be executed in this Runnable
		*/
		template <class TRet,class ErrorType = core::ErrorInfo,class F> 
			Future<TRet,ErrorType> execute( F&& function);
		template <class TRet,class ErrorType = core::ErrorInfo,class F> 
			Future<TRet,ErrorType> execute( F&& function,Future<TRet,ErrorType>);

		/**
		* subscribe to finish event. This event will be executed when Runnable finish,
		* so, when onRun returns;
		*/
		template <class F>
		void subscribeFinishEvent( F&& functor);
				
		/**
		* Checks for a given task to be acomplished
		* @param[in] taskId the task to check
		* @return true if the task has already been executed. false otherwise
		*/
		inline const ProcessScheduler& getScheduler() const;
		inline ProcessScheduler& getScheduler() ;

		/**
		* set timer to be used by internal scheduler
		* If no Timer is given, a new Timer is created first time
		*/
		void setTimer( std::shared_ptr<Timer> timer );
		inline const std::shared_ptr<Timer> getTimer( ) const;
		inline std::shared_ptr<Timer> getTimer( );
		inline unsigned int getPendingTaskCount() const 
		{
			return (unsigned int)getScheduler().getProcessCount();
		}
		/*
		* get number of processes running (not sleeped or paused)
		*/
	//	intentar flexibilizarlo para cuando considere los wait
		inline unsigned int getActiveTaskCount() const 
		{
			return (unsigned int)getScheduler().getActiveProcessCount();
		}
		inline unsigned int getMaxPoolTasks() const{ return mMaxTaskSize;}
	private:

	};
	
	template <class ProcessType,class AllocatorType,class F>
	std::shared_ptr<Process> Runnable::post(
		F&& task_proc,
		bool autoKill,
		// ETaskPriority priority ,
		unsigned int period,unsigned int startTime,bool lockScheduler )
	{
		::std::shared_ptr<ProcessType> p(AllocatorType::allocate(this));
		p->setProcessCallback( ::std::forward<F>(task_proc) );
		p->setPeriod( period );
		p->setAutoKill( autoKill );		
		postTask(p,startTime,lockScheduler);
		return p;
				
		// postTask(nullptr);
		// return nullptr;
	}
	template <class ProcessType ,class AllocatorType, class F>		
	std::shared_ptr<Process> Runnable::fireAndForget(
			F&& f,
			unsigned int startTime,
			bool lockScheduler)
	{
		// return post<ProcessType,AllocatorType>(
		// 	RUNNABLE_CREATETASK
		// 		(
		// 			returnAdaptor<void>
		// 			(
		// 				std::forward<F>(f)
		// 				, ::tasking::EGenericProcessResult::KILL
		// 			)
		// 		),true,0,startTime
		// );
		
		return post<ProcessType,AllocatorType>(
			[f=std::forward<F>(f)](RUNNABLE_TASK_PARAMS) mutable
			{
				f();
				return ::tasking::EGenericProcessResult::KILL;
			}
			,true,0,startTime,lockScheduler
		);
	}

	const ProcessScheduler& Runnable::getScheduler() const
	{
		return mTasks;
	}
	ProcessScheduler& Runnable::getScheduler() 
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
	template <class TRet,class ErrorType, class F> 
	Future<TRet,ErrorType> Runnable::execute( F&& function)
	{
		Future<TRet,ErrorType> future;
		return execute(std::forward<F>(function),future);		
	}
	/**
	 * @brief Overload where output Future is given
	 * With this overload the given Future is fille with result from function. 
	 * @param output Future where result must be put
	 * @return same future as out
	 */
	template <class TRet,class ErrorType , class F> 
	Future<TRet,ErrorType> Runnable::execute( F&& function,Future<TRet,ErrorType> output)
	{
		//always post the task, despite being in same thread. This is the most consistent way of doing it
		post(
				RUNNABLE_CREATETASK
				(
					returnAdaptor<void>
					(
						linkFunctor<void, TYPELIST()>(ExecuteTask<TRet, ErrorType, typename ::std::decay<F>::type>(::std::forward<F>(function)), output)
						, ::tasking::EGenericProcessResult::KILL
						)
				),
				 false  //@todo revisar cómo debería ser
				, 0

			);		
		return output;
	}
	
		
	///@cond HIDDEN_SYMBOLS
	//helper class por request task to Runnable
	template <class TRet,class ErrorType, class F> struct ExecuteTask
	{
		F mFunction;
		ExecuteTask(F&&f):mFunction(std::forward<F>(f))
		{			
		}
		void operator()( /*F& function,*/ Future<TRet,ErrorType> f )
		{
			try
			{
				f.setValue(mFunction());
			}
			//check chances of Exception
			catch( std::exception& e )
			{
				/*Runnable::ExecuteErrorInfo* ei = new Runnable::ExecuteErrorInfo;
				ei->error = Runnable::ERRORCODE_EXCEPTION;
				ei->exc = e.clone();
				ei->isPointer = false;*/
				f.setError( ErrorType(Runnable::ERRORCODE_EXCEPTION,e.what()) );	
			}
			catch(...)
			{
				/*Future_Base::ErrorInfo* ei = new Future_Base::ErrorInfo; 
				ei->error = Runnable::ERRORCODE_UNKNOWN_EXCEPTION;
				ei->errorMsg = "Unknown exception";*/
				f.setError( ErrorType(Runnable::ERRORCODE_UNKNOWN_EXCEPTION,"Unknown exception") );	

			}

		}
		bool operator ==( const ExecuteTask& ) const{ return true;} //for compliance
	};
	//specialization for void TRet
	template <class ErrorType,class F> struct ExecuteTask<void,ErrorType,F>
	{
		F mFunction;
		ExecuteTask(F&&f) :mFunction(std::forward<F>(f))
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
				f.setError( ErrorType(Runnable::ERRORCODE_EXCEPTION,e.what()) );	
			}
			catch(...)
			{
				f.setError( ErrorType(Runnable::ERRORCODE_UNKNOWN_EXCEPTION,"Unknown exception") );	
			}
		}
		bool operator ==( const ExecuteTask& )const { return true;} //for compliance
	};
	///@endcond

}
