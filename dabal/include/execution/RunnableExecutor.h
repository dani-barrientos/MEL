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
            ///@{ 
            //! brief mandatory interface from Executor
            template <class TRet,class TArg,class F,class ErrorType = ::core::ErrorInfo> void launch( F&& f,TArg&& arg,ExFuture<Runnable,TRet,ErrorType> output) const
            {
                if ( !mRunnable.expired())
                {         
                    mRunnable.lock()->execute<TRet>(std::bind(std::forward<F>(f),std::forward<TArg>(arg)),static_cast<Future<TRet,ErrorType>>(output),mOpts.autoKill?Runnable::_killTrue:Runnable::_killFalse);
                }            
            }
            template <class TRet,class F,class ErrorType = ::core::ErrorInfo> void launch( F&& f,ExFuture<Runnable,TRet,ErrorType> output) const
            {
                if ( !mRunnable.expired())
                {
                    mRunnable.lock()->execute<TRet>(std::forward<F>(f),static_cast<Future<TRet,ErrorType>>(output),mOpts.autoKill?Runnable::_killTrue:Runnable::_killFalse);
                }            
            }
            template <class I, class F>	 ::parallelism::Barrier loop(I&& begin, I&& end, F&& functor, int increment);
            template <class TArg,class ...FTypes,class ErrorType = ::core::ErrorInfo> ::parallelism::Barrier parallel(ExFuture<Runnable,TArg,ErrorType> fut, FTypes&&... functions);
            template <class ReturnTuple,class TArg,class ...FTypes,class ErrorType = ::core::ErrorInfo> ::parallelism::Barrier parallel_convert(ExFuture<Runnable,TArg,ErrorType> fut,ReturnTuple& result, FTypes&&... functions);
            ///@}
        private:
            std::weak_ptr<Runnable> mRunnable; 
            RunnableExecutorOpts    mOpts;            
    };  
    namespace _private
    {
        template <class F,class TArg,class ErrorType = ::core::ErrorInfo> void _invoke(ExFuture<Runnable,TArg,ErrorType> fut,::parallelism::Barrier& b,F&& f)
        {
            //@todo no voy a ahcer la gestion de excepciones por ahora que tengo que meditar si realmente debo hacerlo asi
            execution::launch(fut.ex,
                [f = std::forward<F>(f),b](ExFuture<Runnable,TArg,ErrorType> fut) mutable
                {                                        
                    f(fut.getValue().value());
                    b.set();
                },fut);
        }
         //void overload
        template <class F,class ErrorType = ::core::ErrorInfo> void _invoke(ExFuture<Runnable,void,ErrorType> fut,::parallelism::Barrier& b,F&& f)
        {
            execution::launch(fut.ex,
                [f = std::forward<F>(f),b](ExFuture<Runnable,void,ErrorType> fut) mutable
                {                    
                    f();
                    b.set();
                },fut);
        }
        template <class TArg,class F,class ...FTypes,class ErrorType = ::core::ErrorInfo> void _invoke(ExFuture<Runnable,TArg,ErrorType> fut,::parallelism::Barrier& b,F&& f, FTypes&&... fs)
        {            
            _invoke(fut,b,std::forward<F>(f));
            _invoke(fut,b,std::forward<FTypes>(fs)...);
        }
        template <int n,class ResultTuple, class F,class TArg,class ErrorType = ::core::ErrorInfo> void _invoke_with_result(ExFuture<Runnable,TArg,ErrorType> fut,::parallelism::Barrier& b,ResultTuple& output,F&& f)
        {
            execution::launch(fut.ex,
                [f = std::forward<F>(f),b,&output](ExFuture<Runnable,TArg,ErrorType> fut) mutable
                {                                        
                    std::get<n>(output) = f(fut.getValue().value());
                    b.set();
                },fut);
        }
         //void overload
        template <int n,class ResultTuple,class F,class ErrorType = ::core::ErrorInfo> void _invoke_with_result(ExFuture<Runnable,void,ErrorType> fut,::parallelism::Barrier& b,ResultTuple& output, F&& f)
        {
            execution::launch(fut.ex,
                [f = std::forward<F>(f),b,&output](ExFuture<Runnable,void,ErrorType> fut) mutable
                {                    
                    std::get<n>(output) = f();
                    b.set();
                },fut);
        }
        
        template <int n,class ResultTuple,class TArg,class F,class ...FTypes,class ErrorType = ::core::ErrorInfo> void _invoke_with_result(ExFuture<Runnable,TArg,ErrorType> fut,::parallelism::Barrier& b,ResultTuple& output,F&& f, FTypes&&... fs)
        {            
            _invoke_with_result<n>(fut,b,output,std::forward<F>(f));
            _invoke_with_result<n+1>(fut,b,output,std::forward<FTypes>(fs)...);
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
                ptr->fireAndForget(
                    [functor,barrier,i]() mutable
                    {
                        functor(i);
                        barrier.set();
                    },0,autoKill?Runnable::_killTrue:Runnable::_killFalse,!mustLock
                );
        }else
        {
            ptr->fireAndForget(
                    [functor,barrier,begin,end,increment]() mutable
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
    template <class TArg,class ...FTypes,class ErrorType> ::parallelism::Barrier Executor<Runnable>::parallel( ExFuture<Runnable,TArg,ErrorType> fut,FTypes&&... functions)
    {
        ::parallelism::Barrier barrier(sizeof...(functions));
        _private::_invoke(fut,barrier,functions...);
        return barrier;        
    }    
    template <class ReturnTuple,class TArg,class ...FTypes,class ErrorType> ::parallelism::Barrier Executor<Runnable>::parallel_convert(ExFuture<Runnable,TArg,ErrorType> fut,ReturnTuple& result, FTypes&&... functions)
    {
        ::parallelism::Barrier barrier(sizeof...(functions));
        _private::_invoke_with_result<0>(fut,barrier,result,functions...);
        return barrier;        
    }
    typedef Executor<Runnable> RunnableExecutor; //alias
}