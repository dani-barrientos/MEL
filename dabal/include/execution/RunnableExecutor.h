#pragma once
#include <tasking/Runnable.h>
#include <execution/Executor.h>
#include <execution/Continuation.h>
#include <parallelism/Barrier.h>
#include <mpl/TypeTraits.h>
namespace execution
{   
    namespace _private
    {
        template <class TRet,class TArg,class ExecutorType> class ContinuationData;
    }
    template <class TRet,class TArg,class ExecutorType> class Continuation;

    /**
     * @brief Additional hints for this kind of executor
     * @details if loop function receives an objetc of this type, use additional hints
     */
    struct RunnableExecutorLoopHints : public LoopHints
    {
        bool blockOnPost = false; //!<if true,task is posted previous bloking Runnable internal scheduler
    };
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
            template <class I, class F>	 ::parallelism::Barrier loop(I&& begin, I&& end, F&& functor,const LoopHints& hints, int increment = 1);

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
    template <class I, class F>	 ::parallelism::Barrier Executor<Runnable>::loop(I&& begin, I&& end, F&& functor, const LoopHints& hints,int increment)
    {        
        typedef typename std::decay<I>::type DecayedIt;
		constexpr bool isArithIterator = ::mpl::TypeTraits<DecayedIt>::isArith;
        int length;
        if constexpr (isArithIterator)
            length = (end-begin);
        else
            length = std::distance(begin, end);
        int nElements = hints.independentTasks?(length+increment-1)/increment:1; //round-up        
        auto ptr = mRunnable.lock();
        bool mustLock;
        if ( dynamic_cast<const RunnableExecutorLoopHints*>(&hints) )
            mustLock = static_cast<const RunnableExecutorLoopHints&>(hints).blockOnPost;
        else
            mustLock = false;
        if ( mustLock )
            ptr->getScheduler().getLock().enter();
        try
        {
            ::parallelism::Barrier result(nElements);
            if ( hints.independentTasks)
            {        
                for(auto i = begin; i < end;i+=increment)
                    ptr->fireAndForget(
                        [functor,result,i]() mutable
                        {
                            functor(i);
                            result.set();
                        },0,!mustLock
                    );
            }else
            {
                ptr->fireAndForget(
                        [functor,result,begin,end,increment]() mutable
                        {
                            for(auto i = begin; i < end;i+=increment)
                            {
                                functor(i);            
                            }            
                            result.set();
                        },0,!mustLock
                    );           
            }
            if ( mustLock )
                ptr->getScheduler().getLock().leave();
            return result; //@todo resolver
        }catch(...)
        {
            if ( mustLock )
                ptr->getScheduler().getLock().leave();
            return ::parallelism::Barrier();
        }
                    
    }
    typedef Executor<Runnable> RunnableExecutor; //alias
}