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
        
    template <class TArg, class I, class F>	 ExFuture<Runnable,void> loop(ExFuture<Runnable,TArg> fut,I&& begin, I&& end, F&& functor, int increment = 1)
    {        

//@TODODEBO METER EL LOOP COMO MIEMBRO DEL EXECUTOR Y HACERLO COO ALGORIMO. Y ASÍ QUE DEVUELVA UNA BARRERA

        ExFuture<Runnable,void> result(fut.ex);
        fut.subscribeCallback(
            std::function<::core::ECallbackResult( const typename core::FutureValue<TArg>&)>([ex = fut.ex,f = std::forward<F>(functor),result,begin,end,increment](const typename core::FutureValue<TArg>& input)  mutable
            {
                typedef typename std::decay<I>::type DecayedIt;
                constexpr bool isArithIterator = ::mpl::TypeTraits<DecayedIt>::isArith;
                if ( ex.getRunnable().expired())
                {
                    result.setError(core::ErrorInfo(0,"Runnable has been destroyed"));
                    return ::core::ECallbackResult::UNSUBSCRIBE; 
                }
                bool mustLock = ex.getOpts().lockOnce;
                bool autoKill = ex.getOpts().autoKill;
                bool independentTasks = ex.getOpts().independentTasks;
                int length;
                if constexpr (isArithIterator)
                    length = (end-begin);
                else
                    length = std::distance(begin, end);
                int nElements = independentTasks?(length+increment-1)/increment:1; //round-up        
                auto ptr = ex.getRunnable().lock();        
                if ( mustLock )
                    ptr->getScheduler().getLock().enter();
                try
                {
                    ::parallelism::Barrier barrier(nElements);
                    if ( independentTasks)
                    {        
                        for(auto i = begin; i < end;i+=increment)
                            ptr->fireAndForget(
                                [f,barrier,i,input]() mutable
                                {
                                    f(i,input);
                                    barrier.set();
                                },0,autoKill?Runnable::_killTrue:Runnable::_killFalse,!mustLock
                            );
                    }else
                    {
                        ptr->fireAndForget(
                                [f,barrier,begin,end,increment,input]() mutable
                                {
                                    for(auto i = begin; i < end;i+=increment)
                                    {
                                        f(i,input);            
                                    }            
                                    barrier.set();
                                },0,autoKill?Runnable::_killTrue:Runnable::_killFalse,!mustLock
                            );           
                    }
                    if ( mustLock )
                        ptr->getScheduler().getLock().leave();

                    barrier.subscribeCallback(
                        std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([result](const ::parallelism::BarrierData& ) mutable
                        {
                            result.setValue();
                            return ::core::ECallbackResult::UNSUBSCRIBE; 
                        }));

                }catch(...)
                {
                    if ( mustLock )
                        ptr->getScheduler().getLock().leave();
                    result.setValue();
                }
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            })
        );
              
        return result;               
    }
    namespace _private
    {
        template <class F,class TArg> void _invoke(Executor<Runnable> ex,::parallelism::Barrier& b,TArg&& arg,F&& f)
        {
            execution::launch(ex,
                [f = std::forward<F>(f),b](TArg&& arg) mutable
                {
                    f(arg);
                    b.set();
                },arg);
        }
        template <class TArg,class F,class ...FTypes> void _invoke(Executor<Runnable> ex,::parallelism::Barrier& b,TArg&& arg,F&& f, FTypes&&... fs)
        {
            _invoke(ex,b,arg,std::forward<F>(f));
            _invoke(ex,b,arg,std::forward<FTypes>(fs)...);
        }

    }
    template <class TArg,class ...FTypes> ExFuture<Runnable,void> bulk(ExFuture<Runnable,TArg> fut, FTypes... functions)
    {
        ExFuture<Runnable,void> result(fut.ex);

        fut.subscribeCallback(            
            //std::function<::core::ECallbackResult( const typename core::FutureValue<TArg>&)>([ex = fut.ex,result,functions... ](const typename core::FutureValue<TArg>& input)  mutable
            std::function<::core::ECallbackResult( const typename core::FutureValue<TArg>&)>([ex = fut.ex,result,fs = std::make_tuple(std::forward<FTypes>(functions)... )](const typename core::FutureValue<TArg>& input)  mutable
            {
                //::parallelism::Barrier barrier(sizeof...(fs));
                ::parallelism::Barrier barrier(std::tuple_size_v<decltype(fs)>);
                 //_private::_invoke(ex,barrier,input,std::forward<FTypes>(fs)...);
                //_private::_invoke(ex,barrier,input,std::get<FTypes>(fs)...);
                _private::_invoke(ex,barrier,input,std::forward<FTypes>(std::get<FTypes>(fs))...);
                barrier.subscribeCallback(
                    std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([result](const ::parallelism::BarrierData& ) mutable
                    {
                        result.setValue();
                        return ::core::ECallbackResult::UNSUBSCRIBE; 
                    }));
                    
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            }));       
        
        return result;
    }
    typedef Executor<Runnable> RunnableExecutor; //alias
}