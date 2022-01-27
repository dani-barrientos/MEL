#pragma once
#include <tasking/Runnable.h>
#include <execution/Executor.h>
#include <execution/Continuation.h>
namespace execution
{   
    namespace _private
    {
        template <class TRet,class TArg,class ExecutorType> class ContinuationData;
    }
    template <class TRet,class TArg,class ExecutorType> class Continuation;

    /**
     * @brief Executor specialization using Runnable as execution agent
     */
    template <> class Executor<Runnable>
    {
        public:
            Executor(std::shared_ptr<Runnable> runnable):mRunnable(runnable)
            {

            };
            //@todo poder indicar el ErrorType
            template <class TRet,class TArg,class F> Continuation<TRet,TArg,Executor<Runnable>> launch( F&& f,const typename Continuation<TRet,TArg,Executor<Runnable>>::ArgType& arg);
            template <class TRet,class TArg,class F> Continuation<TRet,TArg,Executor<Runnable>> launch( F&& f,TArg arg);
            template <class TRet,class F> Continuation<TRet,void,Executor<Runnable>> launch( F&& f); //@todo resolver el pasar parámetro a la priemra. Esto tiene varios problemas que igual no merece la pena

        private:
            std::weak_ptr<Runnable> mRunnable; 
        public: //@todo hasta resolver friendship, debe ser privado
            template <class TRet,class F> void _execute(F&& f,Future<TRet> output)
            {
                //mRunnable.lock()->fireAndForget(std::forward<F>(f));
                mRunnable.lock()->execute<TRet>(std::forward<F>(f),output);
            }
    };    
    //Executor::launch
    template <class TRet,class TArg,class F> Continuation<TRet,TArg,Executor<Runnable>> Executor<Runnable>::launch( F&& f,const typename Continuation<TRet,TArg,Executor<Runnable>>::ArgType& arg )
    {
        Continuation<TRet,TArg,Executor<Runnable>> result(*this,std::forward<F>(f));
        result._start(arg);
        return result;
    }	
    template <class TRet,class F> Continuation<TRet,void,Executor<Runnable>> Executor<Runnable>::launch( F&& f )
    {
        Continuation<TRet,void,Executor<Runnable>> result(*this,std::forward<F>(f));
        result._start(typename Continuation<TRet,void,Executor<Runnable>>::ArgType());
        return result;
    }	
    template <class TRet,class TArg,class F> Continuation<TRet,TArg,Executor<Runnable>> Executor<Runnable>::launch( F&& f,TArg arg)
    {
        Continuation<TRet,TArg,Executor<Runnable>> result(*this,std::forward<F>(f));
        result._start(arg);
        return result;
    }
    typedef Executor<Runnable> RunnableExecutor; //alias
}