#pragma once
#include <parallelism/ThreadPool.h>
#include <execution/Executor.h>
#include <parallelism/For.h>
namespace execution
{   
    using ::parallelism::ThreadPool;
   
    /**
     * @brief Executor specialization using a ThreadPool as execution agent
     */
    template <> class Executor<ThreadPool>
    {
        public:
            Executor(std::shared_ptr<ThreadPool> pool):mPool(pool)
            {

            };           
            //template <class I, class F>	 ::parallelism::Barrier loop(I&& begin, I&& end, F&& functor,const LoopHints& hints, int increment = 1);
            template <class TRet,class TArg,class F> void launch( F&& f,TArg&& arg,ExFuture<ThreadPool,TRet> output) const
            {
                if ( !mPool.expired())
                {
                    ThreadPool::ExecutionOpts opts;
                    opts.schedPolicy = ThreadPool::SchedulingPolicy::SP_BESTFIT;
                    auto th = mPool.lock()->selectThread(opts);
                    th->execute<TRet>(std::bind(std::forward<F>(f),std::forward<TArg>(arg)),output);
                }            
            }
            template <class TRet,class F> void launch( F&& f,ExFuture<ThreadPool,TRet> output) const
            {
                  if ( !mPool.expired())
                {
                    ThreadPool::ExecutionOpts opts;
                    opts.schedPolicy = ThreadPool::SchedulingPolicy::SP_BESTFIT;
                    auto th = mPool.lock()->selectThread(opts);
                    th->execute<TRet>(std::forward<F>(f),output);               
                }       
            }
    //          template <class I, class F>	 ::parallelism::Barrier Executor<ThreadPool>::loop(I&& begin, I&& end, F&& functor,const LoopHints& hints, int increment)
    // {        
    //     //@todo tratar de generalizar para que estas opciones se puedan indicar..
    //     ThreadPool::ExecutionOpts exopts;
    //     exopts.useCallingThread = false;
    //     exopts.groupTasks = !hints.independentTasks;
    //     return ::parallelism::_for(mPool.lock().get(),exopts,std::forward<I>(begin),std::forward<I>(end),std::forward<F>(functor),increment );
    // }
        private:
            std::weak_ptr<ThreadPool> mPool;        
    };    
  
    typedef Executor<ThreadPool> ThreadPoolExecutor; //alias
}