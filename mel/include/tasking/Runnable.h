#pragma once
/*
 * SPDX-FileCopyrightText: 2005,2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
#include <core/Callback.h>
#include <tasking/ProcessScheduler.h>
#include <tasking/GenericProcess.h>
#include <core/CallbackSubscriptor.h>
using mel::core::CallbackSubscriptor;

#include <map>
using std::map;

using mel::tasking::ProcessScheduler;
using mel::tasking::GenericProcess;

#include <mpl/ParamAdder.h>
using mel::mpl::addParam;
#include <mpl/LinkArgs.h>
using mel::mpl::linkFunctor;
#include <mpl/ReturnAdaptor.h>
using mel::mpl::returnAdaptor;
#include <mpl/MemberEncapsulate.h>
using mel::mpl::makeMemberEncapsulate;
#include <mpl/Functor.h>
using mel::mpl::chain;
#include <mpl/AsPtr.h>
using mel::mpl::asPtr;

#include <core/ThreadDefs.h>

#include <core/Future.h>
using mel::core::Future;
#include <functional>
#include <cassert>
#include <forward_list>
#include <type_traits>

#define RUNNABLE_TASK_ALIGNMENT 8
namespace mel
{
	namespace tasking 
	{		
		//some macros to create task functors easily from bool f() functors
		//note: be carefull with f forma, because if it has soma "," inside, maybe you
		// need to use "coma" (defined somewhere in mpl) or use a extra parenthesis
		// for example, if you want to do RUNNABLE_CREATETASK( link1st<false,void>( xxx ) ) this doesn't compile
		//so you need to do RUNNABLE_CREATETASK( (link1st<false,void>( xxx )) ) or RUNNABLE_CREATETASK( link1st<false coma void>( xxx ) )
		//to show de preprocessor that the ',' is not a parameter separator sign
	#define RUNNABLE_CREATETASK( f ) \
		addParam< ::mel::tasking::EGenericProcessResult,Process*,uint64_t,void > \
		(addParam< ::mel::tasking::EGenericProcessResult,uint64_t,void >( f ) ) 
	//useful macro to declare task parameters
	#define RUNNABLE_TASK_PARAMS uint64_t t,Process* p
		class Runnable; //predeclaration
		/**
		 * @brief Base factory class for tasks
		*/
		class MEL_API ProcessFactory
		{
			public:
				//!\brief Create new process for given Runnable
				inline GenericProcess* create(Runnable* owner)const {return onCreate(owner);};
			protected:
				/**
				 * @brief Reimplement in children to change default behaviour
				 * @details default factory behaviour is to create a _private::RunnableTask. Users can create their own Process type or use their own allocation method
				 * by inheriting from this class and use \ref Runnable::setDefaultFactory, or by providing the custom factorytype in \ref Runnable::post
				 * 
				 */
				virtual GenericProcess* onCreate(Runnable* owner) const;
				
		};
		//!@brief Default allocator for new tasks (through \ref Runnable::post) using the runnable default factory \ref Runnable::getDefaultFactory
		struct MEL_API DefaultAllocator
		{
			//! @brief Allocation function
			static GenericProcess* allocate(Runnable* _this);
		};
		///@cond HIDDEN_SYMBOLS
		namespace _private
		{
			class MEL_API RunnableTask final: public GenericProcess
			{
			public:		
				//throws bad_alloc oif no enoguh memory
				static void* operator new( size_t s,Runnable* owner );
				void operator delete(void* ptr, Runnable*) noexcept;
				static void operator delete( void* ptr ) noexcept;			
				RunnableTask(){}
			private:
			};
			/*
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
			*/
			
			struct RTMemPool; //predeclaration
			struct RTMemBlock
			{
				enum class EMemState:uint8_t { FREE = 0,USED = 1 } ;							
				EMemState	memState = EMemState::FREE;  
				alignas(RUNNABLE_TASK_ALIGNMENT) char task[ sizeof( ::mel::tasking::_private::RunnableTask ) ] ;			
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
		class MEL_API Runnable
		{
		private:
			
		public:			
			struct RunnableInfo
			{
				Runnable* current = nullptr;
			};		
			static const unsigned int DEFAULT_POOL_SIZE = 512;
			//static const unsigned int DEFAULT_MAX_NEW_TASKS = DEFAULT_POOL_SIZE*4;
			
			struct RunnableCreationOptions
			{
				unsigned int maxPoolSize = DEFAULT_POOL_SIZE; //!< maximum initial size for internal process pool. 
				ProcessScheduler::SchedulerOptions schedulerOpts; //!< creation options for internal scheduler
			};
			inline ::mel::core::ThreadId getOwnerThreadId() const { assert(mOwnerThread != 0); return mOwnerThread; }
			/**
			* Manually set the owner thread ID.
			* Usually, the thread ID is automatically set when invoking ::run, but
			* this allows the caller to set it up-front, just in case it's needed.
			* @param tid the owner thread to be set.
			*/
			inline void setOwnerThreadId(mel::core::ThreadId tid) {mOwnerThread=tid;}
			static Runnable* getCurrentRunnable();
			/**
			 * @brief Change default factory used to create task through \ref post and \ref fireAndForget
			 * @warning not multithread-safe. If tasks are being posted while this function is changed, it will reach to unpredictable results
			 */			  
			void setDefaultFactory(ProcessFactory* factory){mDefaultFactory.reset(factory);}
			//! Retrieves the current default factory for tasks
			inline const ProcessFactory* getDefaultFactory() const{return mDefaultFactory.get();}	
		protected:
		private:
		//get info on currently executing Runnable in current thread
			static RunnableInfo* _getCurrentRunnableInfo();
			friend class ::mel::tasking::_private:: RunnableTask;			
			//static thread_local Runnable::RunnableInfo tlCurrentRunnable;
			RunnableInfo* mCurrentInfo;
			std::unique_ptr<ProcessFactory>	mDefaultFactory; //factory to use for allocating tasks if no other given
			ProcessScheduler	mTasks;
			RunnableCreationOptions mOpts;
			mel::tasking::_private::MemZoneList mRTZone;
			//std::atomic<State>	mState;
			std::mutex	mMemPoolCS;		
			mel::core::ThreadId	mOwnerThread;//thread executing Runnable		
			
			//! helper function. Create new pool and append it at front
			mel::tasking::_private::RTMemPool* _addNewPool();
			//! helper function to remove pool. Internally at least one pool remains, so maybe pool is not removed
			void _removePool( ::mel::tasking::_private::RTMemPool* );
			
		protected:
			/**
			* @brief Performs a controlled loop over the internal queue, executing
			* pending tasks.
			* This function must be called always from same place, without altering the stack
			*/
			void processTasks();
			
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
			void postTask(std::shared_ptr<Process> process,unsigned int startTime = 0);

			/**
			* @brief Constructor 
			*/
			Runnable(RunnableCreationOptions opts);
			virtual ~Runnable();

	
			//helper functions to use as the "killFunctor" parameter in post(),fireAndForget(), and execute()
			//!helper function to automatically kill process when receiving kill signal
			static std::function<bool()> killTrue;
			//!helper function to reject kill when receiving kill signal
			static std::function<bool()> killFalse;
			/**
			* Posts a new execution request over a functor
			* The execution is NOT guaranteed to be taken into account inmediatly.
			* By default, a ::mel::tasking::_private::RunnableTask is created, which is intended to be used with a custom memory manager for performance reasons. Users can provide their own 
			* AllocatorType class to change the way the underlying Process is created, either by creating a custom specialization of \ref GenericProcess or/and using a custom memory pool
			@see RTMemPool
			* @param[in] task_proc the functor to be executed. It has signature: bool (unsigned int msecs, Process*)
			* @param[in] killFunction. Functor with signature bool () used when kill is executed while doing function.
			* @param[in] period Milliseconds
			* @param[in] startTime milliseconds to begin task
			* @return the process created for this task
			*/
			template <class AllocatorType = ::mel::tasking::DefaultAllocator, class F,class KF = const std::function<bool()>&>
			std::shared_ptr<Process> post(
				F&& task_proc,
				KF&& killFunction=killFalse,			
				unsigned int period = 0,unsigned int startTime = 0);
			/**
			 * @brief Convenient function to post no periodic task with signature void f()
			 * @details see \ref post for considerations con template parameter AllocatorType
			 * @param[in] task_proc functor with signature void f(void)
			 * @param[in] killFunction. Functor with signature bool () used when kill is executed while doing function.
			 */			
			template <class AllocatorType = ::mel::tasking::DefaultAllocator, class F,class KF = const std::function<bool()>&>
			std::shared_ptr<Process> fireAndForget(
				F&& task_proc,
				unsigned int startTime = 0,
				KF&& killFunction=killTrue);
			/**
			* @brief Executes a function in a context of the Runnable.
			* If this Runnable is in the same thread than caller then, depending on forcepost parameter, the functor
			* will be executed directly (so Future<TRet> will be always available at return) or posted (so caller will
			* need to wait on this Future or whatever other mechanism)
			* @param[in] function Functor with signature TRet f() that will be executed in this Runnable
			* @param[in] killFunction. Functor with signature bool () used when kill is executed while doing function.
			*/
			template <class TRet,class F,class KF = const std::function<bool()>&> 
				Future<TRet> execute( F&& function,KF&& killFunction=killFalse) noexcept;
			template <class TRet,class F,class KF = const std::function<bool()>&> 
				Future<TRet> execute( F&& function,Future<TRet>,KF&& killFunction=killFalse) noexcept;

			
					
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
			inline unsigned int getMaxPoolSize() const{ return mOpts.maxPoolSize;}
		private:

		};
		
		template <class AllocatorType, class F,class KF>
		std::shared_ptr<Process> Runnable::post(
			F&& task_proc,
			KF&& killFunction,
			unsigned int period,unsigned int startTime )
		{
			//@todo para pruebas. Sacaré mejor un warning
			if constexpr ( std::is_nothrow_invocable<F,uint64_t,Process*>::value) 
			{
				//@todo continuar con este temaeste warning no me muestra la variable				
			//	char Runnable_post_task_should_be_noexcept = 300;//"Runnable::post. Task must be noexcept");
				//problema, quiero que saque warning. Una solucion podria ser tener una sobrecarga del post o un parámetro para que ignore el noexcept

			}
			//static_assert( std::is_nothrow_invocable<F,uint64_t,Process*>::value,"Runnable::post. Task must be noexcept");
			::std::shared_ptr<GenericProcess> p(AllocatorType::allocate(this));			
			p->setProcessCallback( ::std::forward<F>(task_proc) );
			p->setPeriod( period );
			p->setKillCallback( ::std::forward<KF>(killFunction) );
			postTask(p,startTime);
			return p;	
		}
		template <class AllocatorType, class F,class KF>
			std::shared_ptr<Process> Runnable::fireAndForget(
				F&& task_proc,
				unsigned int startTime,
				KF&& killFunction)
		{	
			//static_assert( std::is_nothrow_invocable<F>::value,"Runnable::fireAndForget. Task must be noexcept");
			return post<AllocatorType>(
				[f=std::forward<F>(task_proc)](RUNNABLE_TASK_PARAMS) mutable
				{
					f();
					return ::mel::tasking::EGenericProcessResult::KILL;
				}
				,std::forward<KF>(killFunction),0,startTime
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
		
		template <class TRet, class F,class KF> 
		Future<TRet> Runnable::execute( F&& function,KF&& killFunction) noexcept
		{
			Future<TRet> future;	
			return execute(std::forward<F>(function),future,std::forward<KF>(killFunction));		
		}
		/**
		 * @brief Overload where output Future is given
		 * With this overload the given Future is fille with result from function. 
		 * @param output Future where result must be put
		 * @return same future as out
		 */
		template <class TRet, class F,class KF> 
		Future<TRet> Runnable::execute( F&& f,Future<TRet> output,KF&& killFunction) noexcept
		{
			//always post the task, despite being in same thread. This is the most consistent way of doing it

		//PRUEBAS
			// try
			// {
			// 	if constexpr (std::is_same<void,TRet>::value)
			// 	{
			// 		f();
			// 		output.setValue();
			// 	}
			// 	else
			// 		output.setValue(f());
			// }
			// //check chances of Exception
			// catch( std::exception& e )
			// {					
			// 	output.setError( ErrorType(Runnable::ERRORCODE_EXCEPTION,e.what()) );	
			// }
			// catch(...)
			// {
				
			// 	output.setError( ErrorType(Runnable::ERRORCODE_UNKNOWN_EXCEPTION,"Unknown exception") );	

			// }
			
			post(
				[output,f = std::forward<F>(f)](RUNNABLE_TASK_PARAMS) mutable noexcept
				{
					if constexpr (noexcept(f()))
					{
						if constexpr (std::is_same<void,TRet>::value)
						{
							f();
							output.setValue();
						}
						else
						{
							output.setValue(f());
						}
					}else
					{
						try	
						{
							if constexpr (std::is_same<void,TRet>::value)
							{
								f();
								output.setValue();
							}
							else
							{
								output.setValue(f());
							}
						}
						catch(...)
						{
							//output.setError( ErrorType(Runnable::ERRORCODE_UNKNOWN_EXCEPTION,"Unknown exception") );	
							output.setError( std::current_exception() );	
						}
					}
					/*
					//check chances of Exception
					catch( std::exception& e )
					{					
						output.setError( ErrorType(Runnable::ERRORCODE_EXCEPTION,e.what()) );						
					}*/
					
					return ::mel::tasking::EGenericProcessResult::KILL;
				},std::forward<KF>(killFunction)		
			);
			
		
			return output;
		}						
		
	}
}