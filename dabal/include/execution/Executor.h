#pragma once
#include <execution/Continuation.h>
/**
 * @brief High level execution utilities
 * @details This namespace contains class, functions..to give a consisten execution interface independet of the underliying execution system
 */
namespace execution
{    
     /**
     * @brief Hints to pass to Executor::loop
     * 
     */
    struct LoopHints
    {
        bool independentTasks = true; //<! if true, try to make each iteration independent
        virtual ~LoopHints(){} //to make it polymorphic
    };
    template <class ExecutorAgent> class Executor
    {
        //mandatory interface to imlement in specializations
        template <class TRet,class TArg,class F> Continuation<TRet,TArg,Executor<Runnable>> launch( F&& f,const typename Continuation<TRet,TArg,Executor<Runnable>>::ArgType& arg);
        template <class TRet,class TArg,class F> Continuation<TRet,TArg,Executor<Runnable>> launch( F&& f,TArg arg);
        template <class TRet,class F> Continuation<TRet,void,Executor<Runnable>> launch( F&& f); //@todo resolver el pasar parámetro a la priemra. Esto tiene varios problemas que igual no merece la pena
        template <class I, class F>	 ::parallelism::Barrier loop(I&& begin, I&& end, F&& functor,const LoopHints& hints, int increment = 1);
    };

}