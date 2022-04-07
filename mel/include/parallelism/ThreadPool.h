#pragma once
#include <tasking/ThreadRunnable.h>
using mel::tasking::ThreadRunnable;
#include <parallelism/Barrier.h>
using mel::parallelism::Barrier;
#include <mpl/Tuple.h>
using mel::mpl::Tuple;
#include <mpl/_If.h>
using::mel::mpl::_if;
#include <mpl/IsSame.h>
using::mel::mpl::isSame;
//#include <mpl/LinkArgs.h>
#include <mpl/ParamAdder.h>
using mel::mpl::addParam;
#include <functional>
#undef max
#include <limits>
#include <tasking/GenericProcessDefs.h>
/**
* @namespace parallelism
* @brief parallelism support
*/
namespace mel
{
	namespace parallelism
	{
		/**
		 * @brief Pool of threads allowing parallel execution
		 * @details
		 * Code example:
		 * @code {.cpp}
		 * ThreadPool::ThreadPoolOpts opts;
		 * ThreadPool myPool(opts);
		 * ThreadPool::ExecutionOpts exopts;
		 * @endcode
		 */
		class MEL_API ThreadPool
		{
		public:
			enum class SchedulingPolicy {
				SP_ROUNDROBIN,
				SP_BESTFIT,
				SP_EXPLICIT
			};
			constexpr static int THREADS_USE_ALL_CORES = ::std::numeric_limits<int>::max();
			constexpr static uint64_t THREAD_AFFINITY_ALL = -1;
			struct ThreadPoolOpts
			{
				int nThreads = THREADS_USE_ALL_CORES; //number of threads to create or THREADS_USE_ALL_CORES. if negative number, means all available cores minus nThreads (p.e, if 8 available cores and set -2, use 6 cores)
				uint64_t affinity = THREAD_AFFINITY_ALL;  //by default, all cores allowed
				bool forceAffinitty = false; //force each thread to be in a fixed core 
				Runnable::RunnableCreationOptions threadOpts;
			};
			//typedef Callback<void, void> TaskType;

			ThreadPool( const ThreadPoolOpts& opts );
			~ThreadPool();
			inline size_t getNumThreads() const{return mNThreads;}
			struct ExecutionOpts {
				bool useCallingThread = false;
				bool groupTasks = true; //group task when number of tasks is greater than threads. This way, grouped tasks, are execute one after each
				SchedulingPolicy schedPolicy = SchedulingPolicy::SP_ROUNDROBIN;
				size_t threadIndex = 0; //set thread index to use when schedPolicy is SP_EXPLICIT
			};
			template <class TArg,class ... FTypes> void execute(const ExecutionOpts& opts,std::exception_ptr& except, Barrier& barrier,TArg&& arg,FTypes ... functions)
			{
				constexpr int nTasks = sizeof...(functions);
				_execute(opts,except, barrier, std::forward<TArg>(arg),std::forward<FTypes>(functions)...);
			}
			template <class TArg,class ... FTypes> Barrier execute(const ExecutionOpts& opts,std::exception_ptr& except, TArg&& arg,FTypes ... functions)
			{
				constexpr int nTasks = sizeof...(functions);
				Barrier result(nTasks);
				_execute(opts,except,result,std::forward<TArg>(arg),std::forward<FTypes>(functions)...);		
				return result;
			}
			/**
			 * @brief execute given functions and return result in the tuple		 
			 */
			template <class ReturnTuple,class TArg,class ... FTypes> void executeWithResult(const ExecutionOpts& opts,std::exception_ptr& except, Barrier& barrier,ReturnTuple& output,TArg&& arg,FTypes ... functions)
			{
				constexpr int nTasks = sizeof...(functions);
				_executeWithResult<0,ReturnTuple>(opts, except,barrier, output,std::forward<TArg>(arg),std::forward<FTypes>(functions)...);
			}
			template <class ReturnTuple, class TArg,class ... FTypes> Barrier executeWithResult(const ExecutionOpts& opts,std::exception_ptr& except,ReturnTuple& output, TArg&& arg,FTypes ... functions)
			{
				constexpr int nTasks = sizeof...(functions);
				Barrier result(nTasks);
				_executeWithResult<0,ReturnTuple>(opts,except,result,output,std::forward<TArg>(arg),std::forward<FTypes>(functions)...);		
				return result;
			}
			/**
			 * @brief select thread for execution based on given opts
			 * 
			 */
			std::shared_ptr<ThreadRunnable> selectThread(const ExecutionOpts& opts);
		private:
			ThreadPoolOpts mOpts;
			std::shared_ptr<ThreadRunnable>*	mPool;
			unsigned int		mNThreads;
			volatile	int		mLastIndex;  //last thread used
			core::CriticalSection mExceptionLock;  //one lock to proyect all exception set.
			/**
			* execute generic case.
			* @param[in] opts execution options
			* @note because this function doesn't wait for completion, input argument need to be bound and so copied
			* to be able to provide it to the function when this is executed.
			*/
			template <class F,class TArg,class ... FTypes> void _execute(const ExecutionOpts& opts,std::exception_ptr& except, Barrier& output, TArg&& arg,F&& func,FTypes&&... functions)
			{
				/*
			//@todo	tengo que resolver aqui el tema de no bindear el arg..
				if (mNThreads != 0)
				{
					selectThread(opts)->post(
						std::function<tasking::EGenericProcessResult (uint64_t, Process*)>([func, output,arg](uint64_t, Process*) mutable
					{
						func(arg);
						output.set();
						return mel::tasking::EGenericProcessResult::KILL;
					})
					);
				}
				else // no threads in pool, use calling thread
				{
					func(std::forward<TArg>(arg));
					output.set();			
				}*/
				_execute(opts,except, output, arg,std::forward<F>(func));
				_execute(opts,except, output, arg,std::forward<FTypes>(functions)...);
			}		
			//base case
			template <class F,class TArg> void _execute(const ExecutionOpts& opts,std::exception_ptr& except, Barrier& output,TArg&& arg, F&& func)
			{
				if ( opts.useCallingThread || mNThreads == 0 )
				{
					if constexpr (std::is_nothrow_invocable<F,TArg>::value)
					{
						func(arg);
					}else
					{
						try
						{
							func(arg);
						}catch(...)
						{
							core::Lock lck(mExceptionLock);
							if ( !except )
								except = std::current_exception();
						}						
					}
					output.set();
				}
				else
				{
					mLastIndex = _chooseIndex(opts);
					//@todo	tengo que resolver aqui el tema de no bindear el arg..
					//if constexpr (std::is_nothrow_invocable<F,typename std::remove_reference<TArg>::type>::value)
					if constexpr (std::is_nothrow_invocable<F,TArg>::value)
					{
						mPool[mLastIndex]->post(
						std::function<tasking::EGenericProcessResult (uint64_t,Process*)>([func = std::forward<F>(func),output,arg](uint64_t, Process*) mutable
						{
							func(arg);
							output.set();
							return mel::tasking::EGenericProcessResult::KILL;
						})
						);
					}else
					{
						mPool[mLastIndex]->post(
					std::function<tasking::EGenericProcessResult (uint64_t,Process*)>([&except,this,func = std::forward<F>(func),output,arg](uint64_t, Process*) mutable
					{
							try
							{
								func(arg);
							}catch(...)
							{
								core::Lock lck(mExceptionLock);
								if ( !except )
									except = std::current_exception();
							}
							output.set();
							return mel::tasking::EGenericProcessResult::KILL;
					})
					);
					}
				}
			}		
			template <int n,class ReturnTuple,class F,class TArg,class ... FTypes> void _executeWithResult(const ExecutionOpts& opts,std::exception_ptr& except, Barrier& output,ReturnTuple& result, TArg&& arg,F&& func,FTypes&&... functions)
			{
				_executeWithResult<n,ReturnTuple>(opts,except, output,result, arg,std::forward<F>(func));
				_executeWithResult<n+1,ReturnTuple>(opts,except, output,result, arg,std::forward<FTypes>(functions)...);
			}				
			//base case
			template <int n,class ReturnTuple,class F,class TArg> void _executeWithResult(const ExecutionOpts& opts,std::exception_ptr& except, Barrier& output,ReturnTuple& result, TArg&& arg,F&& func)
			{
				if ( opts.useCallingThread || mNThreads == 0 )
				{
					if constexpr (std::is_nothrow_invocable<F,TArg>::value)
					{
						std::get<n>(result) = func(arg);
					}else
					{
						try
						{
							std::get<n>(result) = func(arg);
						}catch(...)
						{
							core::Lock lck(mExceptionLock);
							if ( !except )
								except = std::current_exception();
						}		
					}
					output.set();
				}
				else
				{
					mLastIndex = _chooseIndex(opts);
					if constexpr (std::is_nothrow_invocable<F,TArg>::value)
					{
						mPool[mLastIndex]->post(
						std::function<tasking::EGenericProcessResult (uint64_t,Process*)>([func = std::forward<F>(func),output,arg,&result](uint64_t, Process*) mutable
						{
							std::get<n>(result) = func(arg);
							output.set();
							return mel::tasking::EGenericProcessResult::KILL;
						})
						);
					}else
					{
						mPool[mLastIndex]->post(
						std::function<tasking::EGenericProcessResult (uint64_t,Process*)>([&except,this,func = std::forward<F>(func),output,arg,&result](uint64_t, Process*) mutable
						{
							try
							{
								std::get<n>(result) = func(arg);
							}catch(...)
							{
								core::Lock lck(mExceptionLock);
								if ( !except )
									except = std::current_exception();
							}		
							output.set();
							return mel::tasking::EGenericProcessResult::KILL;
						})
						);
					}
					
				}
			}	
			size_t _chooseIndex(const ExecutionOpts& sp);
		};
		
	}
}
 