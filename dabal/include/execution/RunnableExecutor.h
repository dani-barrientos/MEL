#pragma once
#include <tasking/Runnable.h>
#include <execution/Executor.h>
//#include <execution/Continuation.h>
#include <parallelism/Barrier.h>
#include <mpl/TypeTraits.h>
namespace execution
{   
    
    
    /**
     * @brief Additional hints for this kind of executor
     * @details if loop function receives an objetc of this type, use additional hints
     */
    struct RunnableExecutorLoopHints : public LoopHints
    {
        //opcion temporal, espero poder quitarla
        bool lockOnce = false; //!<if true,Runnable internal scheduler lock is taken before posting loop taksks
        bool autoKill = true; //!<if true, tasks
    };
    /**
     * @brief Additional hints for launch in this kind of executor
     * 
     */

    struct RunnableLaunchHints : public LaunchHints
    {
      bool autoKill = true; //launched task wil be autokilled or not when if receive kill signal  
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
            // //@todo poder indicar el ErrorType
            // template <class TRet,class TArg,class F> Continuation<TRet,TArg,Executor<Runnable>> launch( F&& f,const typename Continuation<TRet,TArg,Executor<Runnable>>::ArgType& arg,const RunnableLaunchHints& hints = sDefaultHints);
            // template <class TRet,class TArg,class F> Continuation<TRet,TArg,Executor<Runnable>> launch( F&& f,typename Continuation<TRet,TArg,Executor<Runnable>>::ArgType&&,const RunnableLaunchHints& hints = sDefaultHints);
            // template <class TRet,class TArg,class F> Continuation<TRet,TArg,Executor<Runnable>> launch( F&& f,TArg&& arg,const RunnableLaunchHints& hints = sDefaultHints);
            // template <class TRet,class F> Continuation<TRet,void,Executor<Runnable>> launch( F&& f,const RunnableLaunchHints& hints = sDefaultHints); //@todo resolver el pasar parámetro a la priemra. Esto tiene varios problemas que igual no merece la pena
            // template <class I, class F>	 ::parallelism::Barrier loop(I&& begin, I&& end, F&& functor,const LoopHints& hints, int increment = 1);
            inline const std::weak_ptr<Runnable>& getRunnable()const { return mRunnable;}
            inline std::weak_ptr<Runnable>& getRunnable() { return mRunnable;}
            template <class TRet,class TArg,class F> void launch( F&& f,TArg&& arg,ExFuture<Runnable,TRet> output) const
            {
                if ( !mRunnable.expired())
                {
                    mRunnable.lock()->execute<TRet>(std::bind(std::forward<F>(f),std::forward<TArg>(arg)),static_cast<Future<TRet>>(output));
                }            
            }
            template <class TRet,class F> void launch( F&& f,ExFuture<Runnable,TRet> output) const
            {
                if ( !mRunnable.expired())
                {
                    mRunnable.lock()->execute<TRet>(std::forward<F>(f),static_cast<Future<TRet>>(output));
                }            
            }
        private:
            std::weak_ptr<Runnable> mRunnable; 
            static RunnableLaunchHints sDefaultHints;
            /*template <class TRet,class F> void _execute(F&& f,Future<TRet> output)
            {
                if ( !mRunnable.expired())
                    mRunnable.lock()->execute<TRet>(std::forward<F>(f),output);
            }*/
            
    };    
    RunnableLaunchHints Executor<Runnable>::sDefaultHints;   
    //Executor::launch
    /*template <class TRet,class TArg,class F> Future<TRet> launch(Executor<Runnable> ex, F&& f,const typename Continuation<TRet,TArg,Executor<Runnable>>::ArgType& arg )
    {
        Continuation<TRet,TArg,Executor<Runnable>> result(ex,std::forward<F>(f));
        result._start(arg);
        return result;
    }	
    template <class TRet,class TArg,class F> Continuation<TRet,TArg,Executor<Runnable>> launch( Executor<Runnable> ex, F&& f,typename Continuation<TRet,TArg,Executor<Runnable>>::ArgType&& arg )
    {
        Continuation<TRet,TArg,Executor<Runnable>> result(ex,std::forward<F>(f));
        result._start(std::move(arg));
        return result;
    }*/	
    // namespace _private
    // {
    //     template <class TRet,class TArg,class F> void _launch( Executor<Runnable> ex,F&& f,TArg&& arg,Future<TRet> output)
    //     {
    //         if ( !ex.getRunnable().expired())
    //         {
    //             ex.getRunnable().lock()->execute<TRet>(std::bind(std::forward<F>(f),std::forward<TArg>(arg)),output);
    //         }            
    //     }
    //     template <class TRet,class F> void _launch( Executor<Runnable> ex,F&& f,Future<TRet> output)
    //     {
    //         if ( !ex.getRunnable().expired())
    //         {
    //             ex.getRunnable().lock()->execute<TRet>(std::forward<F>(f),output);
    //         }            
    //     }
    // }
    // template <class TRet,class F> Future<TRet> launch( Executor<Runnable> ex, F&& f )
    // {
    //     Future<TRet> result(ex,std::forward<F>(f));
    //     result._start(typename Continuation<TRet,void,Executor<Runnable>>::ArgType());
    //     return result;
    // }	
  
//@todo    mi duda aqui es lo de pasar el executor. Por un lado es lo mejor, es más flexible, pero tenog miedo que se pierda el executor en algún momento
// @todo este algortmopo deberái ser genérico, así que apsarlo a Runnable
    
    /*
    template <class I, class F>	 ::parallelism::Barrier loop(Executor<Runnable> ex,I&& begin, I&& end, F&& functor, const LoopHints& hints,int increment)
    {        
        typedef typename std::decay<I>::type DecayedIt;
		constexpr bool isArithIterator = ::mpl::TypeTraits<DecayedIt>::isArith;
        if ( mRunnable.expired())
            return ::parallelism::Barrier((size_t)0);
        int length;
        if constexpr (isArithIterator)
            length = (end-begin);
        else
            length = std::distance(begin, end);
        int nElements = hints.independentTasks?(length+increment-1)/increment:1; //round-up        
        auto ptr = mRunnable.lock();
        bool mustLock;
        bool autoKill;
        if ( dynamic_cast<const RunnableExecutorLoopHints*>(&hints) )
        {
            mustLock = static_cast<const RunnableExecutorLoopHints&>(hints).lockOnce;
            autoKill = static_cast<const RunnableExecutorLoopHints&>(hints).autoKill;
        }
        else
        {
            mustLock = false;
            autoKill = true;
        }
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
                        },0,autoKill?Runnable::_killTrue:Runnable::_killFalse,!mustLock
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
                        },0,autoKill?Runnable::_killTrue:Runnable::_killFalse,!mustLock
                    );           
            }
            if ( mustLock )
                ptr->getScheduler().getLock().leave();launch<NewRet,TRet>(f,mResult.getValue());
        {
            if ( mustLock )
                ptr->getScheduler().getLock().leave();
            return ::parallelism::Barrier();
        }
                    
    }
    */
    typedef Executor<Runnable> RunnableExecutor; //alias
}