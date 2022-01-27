#pragma once
#include <parallelism/ThreadPool.h>
#include <execution/Executor.h>
#include <execution/Continuation.h>
namespace execution
{   
    using ::parallelism::ThreadPool;
    namespace _private
    {
        template <class TRet,class TArg,class ExecutorType> class ContinuationData;
    }
    template <class TRet,class TArg,class ExecutorType> class Continuation;

    /**
     * @brief Executor specialization using a ThreadPool as execution agent
     */
    template <> class Executor<ThreadPool>
    {
        public:
            Executor(std::shared_ptr<ThreadPool> pool):mPool(pool)
            {

            };
            //@todo poder indicar el ErrorType
            template <class TRet,class TArg,class F> Continuation<TRet,TArg,Executor<ThreadPool>> launch( F&& f,const typename Continuation<TRet,TArg,Executor<ThreadPool>>::ArgType& arg);
            template <class TRet,class TArg,class F> Continuation<TRet,TArg,Executor<ThreadPool>> launch( F&& f,TArg arg);
            template <class TRet,class F> Continuation<TRet,void,Executor<ThreadPool>> launch( F&& f); //@todo resolver el pasar parámetro a la priemra. Esto tiene varios problemas que igual no merece la pena

        private:
            std::weak_ptr<ThreadPool> mPool; 
        public: //@todo hasta resolver friendship, debe ser privado
            template <class TRet,class F> void _execute(F&& f,Future<TRet> output)
            {
                //mRunnable.lock()->execute<TRet>(std::forward<F>(f),output);
                ThreadPool::ExecutionOpts opts;
                opts.schedPolicy = ThreadPool::SchedulingPolicy::SP_BESTFIT;
                auto th = mPool.lock()->selectThread(opts);
                th->execute<TRet>(std::forward<F>(f),output);
            }
    };    
    //Executor::launch
    template <class TRet,class TArg,class F> Continuation<TRet,TArg,Executor<ThreadPool>> Executor<ThreadPool>::launch( F&& f,const typename Continuation<TRet,TArg,Executor<ThreadPool>>::ArgType& arg )
    {
        Continuation<TRet,TArg,Executor<ThreadPool>> result(*this,std::forward<F>(f));
        result._start(arg);
        return result;
    }	
    template <class TRet,class F> Continuation<TRet,void,Executor<ThreadPool>> Executor<ThreadPool>::launch( F&& f )
    {
        Continuation<TRet,void,Executor<ThreadPool>> result(*this,std::forward<F>(f));
        result._start(typename Continuation<TRet,void,Executor<ThreadPool>>::ArgType());
        return result;
    }	
    template <class TRet,class TArg,class F> Continuation<TRet,TArg,Executor<ThreadPool>> Executor<ThreadPool>::launch( F&& f,TArg arg)
    {
        Continuation<TRet,TArg,Executor<ThreadPool>> result(*this,std::forward<F>(f));
        result._start(arg);
        return result;
    }
    typedef Executor<ThreadPool> ThreadPoolExecutor; //alias
}