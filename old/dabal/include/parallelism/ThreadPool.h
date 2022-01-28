#pragma once
#include <core/Thread.h>
using core::Thread;
#include <parallelism/SimpleBarrier.h>
using parallelism::SimpleBarrier;
#include <mpl/Tuple.h>
using mpl::Tuple;
#include <mpl/_If.h>
using ::mpl::_if;
#include <mpl/IsSame.h>
using ::mpl::isSame;
//#include <mpl/LinkArgs.h>
#include <mpl/ParamAdder.h>
using mpl::addParam;
#include <functional>
#undef max
#include <limits>
namespace parallelism
{
	class DABAL_API ThreadPool
	{
	public:
		enum SchedulingPolicy {
			SP_ROUNDROBIN,
			SP_BESTFIT,
			SP_EXPLICIT
		};
		constexpr static int THREADS_USE_ALL_CORES = ::std::numeric_limits<int>::max();
		constexpr static uint64_t THREAD_AFFINITY_ALL = -1;
		struct ThreadPoolOpts
		{
			int nThreads; //number of threads to create or THREADS_USE_ALL_CORES. if negative number, means all available cores minus nThreads (p.e, if 8 available cores and set -2, use 6 cores)
			uint64_t affinity = THREAD_AFFINITY_ALL;  //by default, all cores allowed
			bool forceAffinitty = false; //force each thread to be in a fixed core 
		};
		//typedef Callback<void, void> TaskType;

		ThreadPool( const ThreadPoolOpts& opts );
		~ThreadPool();
		inline size_t getNumThreads() const
		{
			return mNThreads;
		}
		struct ExecutionOpts {
			bool useCallingThread = false;
			bool groupTasks = true; //group task when number of tasks is greater than threads. This way, grouped tasks, are execute one after each
			SchedulingPolicy schedPolicy = SP_ROUNDROBIN;
			size_t threadIndex = 0; //set thread index to use when schedPolicy is SP_EXPLICIT
		};
		template <class ... FTypes> void execute(const ExecutionOpts& opts, SimpleBarrier& barrier,bool updateWorkers, FTypes ... functions)
		{
			constexpr int nTasks = sizeof...(functions);
			if (updateWorkers)
				barrier.addWorkers(nTasks); //update workers using barrier
			//_execute(opts, barrier, std::forward<FTypes>(functions)...);
			_execute(opts, barrier, functions...);
		}
		template <class ... FTypes> SimpleBarrier execute(const ExecutionOpts& opts, FTypes ... functions)
		{
			constexpr int nTasks = sizeof...(functions);
			SimpleBarrier result(nTasks);
		//	std::tuple<FTypes...> args{ std::forward<FTypes>(functions)... };			
			_execute(opts,result,std::forward<FTypes>(functions)...);		
			return result;
		}
	private:
		ThreadPoolOpts mOpts;
		Thread**	mPool;
		unsigned int		mNThreads;
		volatile	int		mLastIndex;  //last thread used
		/**
		* execute generic case.
		* @param[in] opts execution options
		*/
		template <class F,class ... FTypes> void _execute(const ExecutionOpts& opts, SimpleBarrier& output, F&& func,FTypes ... functions)
		{
			if (mNThreads != 0)
			{
				mLastIndex = _chooseIndex(opts);
				mPool[mLastIndex]->post(
					std::function<bool(unsigned int, Process*)>([func, output](unsigned int, Process*) mutable
				{
					func();
					output.set();
					return true;
				})
				);
			}
			else // no threads in pool, use calling thread
			{
				func();
				output.set();

			}
			_execute(opts, output, std::forward<FTypes>(functions)...);
		}
		//base case
		template <class F> void _execute(const ExecutionOpts& opts,  SimpleBarrier& output, F&& func)
		{
			if ( opts.useCallingThread || mNThreads == 0 )
			{
                func();
                output.set();
			}
			else
			{
				mLastIndex = _chooseIndex(opts);
				mPool[mLastIndex]->post(
                   std::function<bool(unsigned int,Process*)>([func,output](unsigned int, Process*) mutable
                   {
                       func();
                       output.set();
                       return true;
                   })
				);
				//mLastIndex = (int)thIdx;
			}
		}
		
		
		
		size_t _chooseIndex(const ExecutionOpts& sp);
	};
	
}
 