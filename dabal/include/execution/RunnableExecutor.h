#pragma once
#include <tasking/Runnable.h>
#include <execution/Executor.h>
//#include <execution/Continuation.h>
#include <parallelism/Barrier.h>
#include <mpl/TypeTraits.h>
namespace execution
{   
    
    
    /**
     * @brief Concrete options for this type of executor
     */
    struct RunnableExecutorOpts
    {
        bool independentTasks = true; //<! if true, try to make each iteration independent
        //opcion temporal, espero poder quitarla
        bool lockOnce = false; //!<if true,Runnable internal scheduler lock is taken before posting loop taksks
        bool autoKill = true; //!<if true, launched tasks will be autokilled if the Runnable receives a kill signal, else, Runanble won't finish until tasks finished
    };
    /**
     * @brief Executor specialization using Runnable as execution agent
     */
    template <> class Executor<Runnable>
    {
        public:
            Executor(Executor&& ex):mRunnable(std::move(ex.mRunnable)),mOpts(ex.mOpts){}
            Executor(const Executor& ex):mRunnable(ex.mRunnable),mOpts(ex.mOpts){}
            Executor(std::shared_ptr<Runnable> runnable):mRunnable(runnable){};
            Executor& operator=(const Executor& ex){mRunnable = ex.mRunnable;mOpts = ex.mOpts;return *this;}
            Executor& operator=( Executor&& ex){mRunnable = std::move(ex.mRunnable);mOpts = ex.mOpts;return *this;}
            void setOpts(const RunnableExecutorOpts& opts){ mOpts = opts;}
            const RunnableExecutorOpts& getOpts(){ return mOpts;}
            // //@todo poder indicar el ErrorType            
            inline const std::weak_ptr<Runnable>& getRunnable()const { return mRunnable;}
            inline std::weak_ptr<Runnable>& getRunnable() { return mRunnable;}
            template <class TRet,class TArg,class F> void launch( F&& f,TArg&& arg,ExFuture<Runnable,TRet> output) const
            {
                if ( !mRunnable.expired())
                {
                    mRunnable.lock()->execute<TRet>(std::bind(std::forward<F>(f),std::forward<TArg>(arg)),static_cast<Future<TRet>>(output),mOpts.autoKill?Runnable::_killTrue:Runnable::_killFalse);
                }            
            }
            template <class TRet,class F> void launch( F&& f,ExFuture<Runnable,TRet> output) const
            {
                if ( !mRunnable.expired())
                {
                    mRunnable.lock()->execute<TRet>(std::forward<F>(f),static_cast<Future<TRet>>(output),mOpts.autoKill?Runnable::_killTrue:Runnable::_killFalse);
                }            
            }
        private:
            std::weak_ptr<Runnable> mRunnable; 
            RunnableExecutorOpts    mOpts;            
    };        
      
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