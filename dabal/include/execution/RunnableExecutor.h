#pragma once
#include <tasking/Runnable.h>
#include <execution/Executor.h>
//#include <execution/Continuation.h>
#include <parallelism/Barrier.h>
#include <mpl/TypeTraits.h>
namespace execution
{   
    
    using tasking::Runnable;
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
            inline const std::weak_ptr<Runnable>& getRunnable()const { return mRunnable;}
            inline std::weak_ptr<Runnable>& getRunnable() { return mRunnable;}
            ///@{ 
            //! brief mandatory interface from Executor
            template <class TRet,class TArg,class F> void launch( F&& f,TArg&& arg,ExFuture<Runnable,TRet> output) const noexcept
            {
                if ( !mRunnable.expired())
                {         
                    //it seems that noexcept specifier is not preserved in bind, so need to use a lambda
                   //mRunnable.lock()->execute<TRet>(std::bind(std::forward<F>(f),std::forward<TArg>(arg)),static_cast<Future<TRet>>(output),mOpts.autoKill?Runnable::_killTrue:Runnable::_killFalse);
                  //if constexpr (noexcept(f(arg)))
                    mRunnable.lock()->execute<TRet>(
                        [f = std::forward<F>(f),arg = std::forward<TArg>(arg)]() mutable noexcept(std::is_nothrow_invocable<F,TArg>::value) ->TRet
                        {                            
                            return f(arg);
                        },
                        static_cast<Future<TRet>>(output)
                    ,mOpts.autoKill?Runnable::_killTrue:Runnable::_killFalse);
                }          
            }
            template <class TRet,class F> void launch( F&& f,ExFuture<Runnable,TRet> output) const noexcept
            {
                if ( !mRunnable.expired())
                {
                    mRunnable.lock()->execute<TRet>(std::forward<F>(f),static_cast<Future<TRet>>(output),mOpts.autoKill?Runnable::_killTrue:Runnable::_killFalse);                   
                }            
            }
            template <class I, class F>	 ::parallelism::Barrier loop( I&& begin, I&& end, F&& functor, int increment);
            template <class TArg,class ...FTypes> ::parallelism::Barrier parallel(ExFuture<Runnable,TArg> fut,std::exception_ptr& excpt, FTypes&&... functions);
            template <class ReturnTuple,class TArg,class ...FTypes> ::parallelism::Barrier parallel_convert(ExFuture<Runnable,TArg> fut,std::exception_ptr& excpt,ReturnTuple& result, FTypes&&... functions);
            ///@}
        private:
            std::weak_ptr<Runnable> mRunnable; 
            RunnableExecutorOpts    mOpts;            
    };  
    namespace _private
    {
        template <class F,class TArg> void _invoke(ExFuture<Runnable,TArg> fut,::parallelism::Barrier& b,std::exception_ptr& except,F&& f)
        {
            if constexpr (std::is_nothrow_invocable<F,TArg&>::value)
            {
                //use the exception pointer hasn't sense here because noexcept was specified
                execution::launch(fut.agent,
                    [f = std::forward<F>(f),b](ExFuture<Runnable,TArg>& fut) mutable noexcept
                    {                                        
                        f(fut.getValue().value());
                        b.set();
                    },fut);
            }else
            {
                execution::launch(fut.agent,
                [f = std::forward<F>(f),b,&except](ExFuture<Runnable,TArg>& fut) mutable
                {                               
                    try
                    {
                        f(fut.getValue().value());
                    }catch(...)
                    {
                        if ( !except )
                            except = std::current_exception();
                    }
                    b.set();
                },fut);
            }
        }
         //void overload
        template <class F> void _invoke(ExFuture<Runnable,void> fut,::parallelism::Barrier& b,std::exception_ptr& except,F&& f)
        {
            if constexpr (std::is_nothrow_invocable<F>::value)
            {
                execution::launch(fut.agent,
                    [f = std::forward<F>(f),b](ExFuture<Runnable,void>& fut) mutable noexcept
                    {                    
                        f();
                        b.set();
                    },fut);
            }else
            {
                execution::launch(fut.agent,
                    [f = std::forward<F>(f),b,&except](ExFuture<Runnable,void>& fut) mutable
                    {               
                        try
                        {
                            f();
                        }catch(...)
                        {
                            if (!except)
                                except = std::current_exception();
                        }                        
                        b.set();
                    },fut);
            }
        }
        template <class TArg,class F,class ...FTypes> void _invoke(ExFuture<Runnable,TArg> fut,::parallelism::Barrier& b,std::exception_ptr& except,F&& f, FTypes&&... fs)
        {            
            _invoke(fut,b,except,std::forward<F>(f));
            _invoke(fut,b,except,std::forward<FTypes>(fs)...);
        }
        template <int n,class ResultTuple, class F,class TArg> void _invoke_with_result(ExFuture<Runnable,TArg> fut,::parallelism::Barrier& b,std::exception_ptr& except,ResultTuple& output,F&& f)
        {
            if constexpr (std::is_nothrow_invocable<F,TArg&>::value)
            {
                execution::launch(fut.agent,
                    [f = std::forward<F>(f),b,&output](ExFuture<Runnable,TArg>& fut) mutable noexcept
                    {                                        
                        std::get<n>(output) = f(fut.getValue().value());
                        b.set();
                    },fut);
            }else
            {
                execution::launch(fut.agent,
                    [f = std::forward<F>(f),b,&output,&except](ExFuture<Runnable,TArg>& fut) mutable
                    {            
                        try
                        {                            
                            std::get<n>(output) = f(fut.getValue().value());
                        }catch(...)
                        {
                            if (!except)
                                except = std::current_exception();
                        }
                        b.set();
                    },fut);
            }
        }
         //void overload
        template <int n,class ResultTuple,class F> void _invoke_with_result(ExFuture<Runnable,void>& fut,::parallelism::Barrier& b,std::exception_ptr& except,ResultTuple& output, F&& f)
        {
            if constexpr (std::is_nothrow_invocable<F>::value)
            {
                execution::launch(fut.agent,
                    [f = std::forward<F>(f),b,&output](ExFuture<Runnable,void>& fut) mutable noexcept
                    {                    
                        std::get<n>(output) = f();
                        b.set();
                    },fut);
            }else
            {
                 execution::launch(fut.agent,
                    [f = std::forward<F>(f),b,&output,&except](ExFuture<Runnable,void>& fut) mutable
                    {   
                        try
                        {                 
                            std::get<n>(output) = f();
                        }catch(...)
                        {
                            if (!except)
                                except = std::current_exception();
                        }

                        b.set();
                    },fut);
            }
        }
        
        template <int n,class ResultTuple,class TArg,class F,class ...FTypes> void _invoke_with_result(ExFuture<Runnable,TArg> fut,::parallelism::Barrier& b,std::exception_ptr& except,ResultTuple& output,F&& f, FTypes&&... fs)
        {            
            _invoke_with_result<n>(fut,b,except,output,std::forward<F>(f));
            _invoke_with_result<n+1>(fut,b,except,output,std::forward<FTypes>(fs)...);
        }
    }      
    /**
     * @brief Concurrent loop
     * Excutes given number of iterations of given functor as independent tasks (up to executor is able to do )
     * 
     * @param fut 
     * @param begin 
     * @param end 
     * @param functor 
     * @param increment 
     * @return en new Future whose value is moved from input future
     */
    template <class I, class F>	 ::parallelism::Barrier Executor<Runnable>::loop(I&& begin, I&& end, F&& functor, int increment)
    {
        typedef typename std::decay<I>::type DecayedIt;
        constexpr bool isArithIterator = ::mpl::TypeTraits<DecayedIt>::isArith;
        if ( getRunnable().expired())
        {
            throw std::runtime_error("Runnable has been destroyed");
        }
        bool mustLock = getOpts().lockOnce;
        bool autoKill = getOpts().autoKill;
        bool independentTasks = getOpts().independentTasks;
        int length;
        if constexpr (isArithIterator)
            length = (end-begin);
        else
            length = std::distance(begin, end);
        int nElements = independentTasks?(length+increment-1)/increment:1; //round-up        
        auto ptr = getRunnable().lock();        
        if ( mustLock )
            ptr->getScheduler().getLock().enter();
    
        ::parallelism::Barrier barrier(nElements);
        if ( independentTasks)
        {                    
            for(auto i = begin; i < end;i+=increment)
            {
                ptr->fireAndForget(
                    [functor,barrier,i]() mutable noexcept((std::is_nothrow_invocable<F,I>::value))
                    {
                        functor(i);
                        barrier.set();
                    },0,autoKill?Runnable::_killTrue:Runnable::_killFalse,!mustLock
                );
            }
        }else
        {
            ptr->fireAndForget(
                    [functor,barrier,begin,end,increment]() mutable noexcept(std::is_nothrow_invocable<F,I>::value)
                    {
                        for(auto i = begin; i < end;i+=increment)
                        {
                            functor(i);            
                        }            
                        barrier.set();
                    },0,autoKill?Runnable::_killTrue:Runnable::_killFalse,!mustLock
                );           
        }
        if ( mustLock )
            ptr->getScheduler().getLock().leave();
        return barrier;
    }
    template <class TArg,class ...FTypes> ::parallelism::Barrier Executor<Runnable>::parallel( ExFuture<Runnable,TArg> fut,std::exception_ptr& except,FTypes&&... functions)
    {
        ::parallelism::Barrier barrier(sizeof...(functions));
        _private::_invoke(fut,barrier,except,functions...);
        return barrier;        
    }    
    template <class ReturnTuple,class TArg,class ...FTypes> ::parallelism::Barrier Executor<Runnable>::parallel_convert(ExFuture<Runnable,TArg> fut,std::exception_ptr& except,ReturnTuple& result, FTypes&&... functions)
    {
        ::parallelism::Barrier barrier(sizeof...(functions));
        _private::_invoke_with_result<0>(fut,barrier,except,result,functions...);
        return barrier;        
    }
    typedef Executor<Runnable> RunnableExecutor; //alias
}