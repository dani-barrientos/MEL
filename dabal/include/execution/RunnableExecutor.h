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
            template <class TArg,class ...FTypes,class ErrorType = ::core::ErrorInfo> ::parallelism::Barrier bulk(ExFuture<Runnable,TArg,ErrorType> fut, FTypes&&... functions);
            ///@}
        private:
            std::weak_ptr<Runnable> mRunnable; 
            RunnableExecutorOpts    mOpts;            
    };  
    namespace _private
    {
        template <class F,class TArg,class ErrorType = ::core::ErrorInfo> void _invoke(ExFuture<Runnable,TArg,ErrorType> fut,::parallelism::Barrier& b,F&& f)
        {
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
        /*//----- pruebas
        template <int n,class F,class TArg,class OutputTuple> void _invoke_debug(Executor<Runnable> ex,::parallelism::Barrier& b,OutputTuple* output,TArg&& arg,F&& f)
        {
            execution::launch(ex,
                [f = std::forward<F>(f),b,output](TArg&& arg) mutable
                {
                    std::get<n>(*output) = f(arg);
                    b.set();                    
                },arg);
        }
        template <int n,class TArg,class OutputTuple,class F,class ...FTypes> void _invoke_debug(Executor<Runnable> ex,::parallelism::Barrier& b,OutputTuple* output,TArg&& arg,F&& f, FTypes&&... fs)
        {
            _invoke_debug<n>(ex,b,output,arg,std::forward<F>(f));
            _invoke_debug<n+1>(ex,b,output,arg,std::forward<FTypes>(fs)...);
        }
*/
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
    template <class TArg,class ...FTypes,class ErrorType = ::core::ErrorInfo> ::parallelism::Barrier Executor<Runnable>::bulk( ExFuture<Runnable,TArg,ErrorType> fut,FTypes&&... fs)
    {
        ::parallelism::Barrier barrier(sizeof...(fs));
        //_private::_invoke(fut,barrier,std::forward<FTypes>(std::get<FTypes>(fs))...);
        _private::_invoke(fut,barrier,fs...);
        return barrier;
        
    }
    /*
    template <class TArg, class I, class F>	 ExFuture<Runnable,TArg> loop(ExFuture<Runnable,TArg> fut,I&& begin, I&& end, F&& functor, int increment = 1)
    {        
//@TODODEBO METER EL LOOP COMO MIEMBRO DEL EXECUTOR Y HACERLO COMO ALGORIMO. Y ASÍ QUE DEVUELVA UNA BARRERA
        ExFuture<Runnable,TArg> result(fut.ex);
        typedef typename ExFuture<Runnable,TArg>::ValueType  ValueType;
        fut.subscribeCallback(
            std::function<::core::ECallbackResult( ValueType&)>([fut,f = std::forward<F>(functor),result,begin,end,increment](ValueType& input)  mutable
            {
                typedef typename std::decay<I>::type DecayedIt;
                constexpr bool isArithIterator = ::mpl::TypeTraits<DecayedIt>::isArith;
                if ( fut.ex.getRunnable().expired())
                {
                    result.setError(core::ErrorInfo(0,"Runnable has been destroyed"));
                    return ::core::ECallbackResult::UNSUBSCRIBE; 
                }
                bool mustLock = fut.ex.getOpts().lockOnce;
                bool autoKill = fut.ex.getOpts().autoKill;
                bool independentTasks = fut.ex.getOpts().independentTasks;
                int length;
                if constexpr (isArithIterator)
                    length = (end-begin);
                else
                    length = std::distance(begin, end);
                int nElements = independentTasks?(length+increment-1)/increment:1; //round-up        
                auto ptr = fut.ex.getRunnable().lock();        
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
                        std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([result,fut](const ::parallelism::BarrierData& ) mutable
                        {
                            //@TODO CREO NO PUEDO USAR EL INPUT BINDEADO, HARía COPia
                            result.assign(std::move(fut.getValue()));
                            return ::core::ECallbackResult::UNSUBSCRIBE; 
                        }));

                }catch(...)
                {
                    if ( mustLock )
                        ptr->getScheduler().getLock().leave();
                    result.setError(::core::ErrorInfo(0,"RunnableExecutor::loop. Unknown error"));
                }
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            })
        );
              
        return result;               
    }*/
    
   /* template <class ReturnTuple,class TArg,class ...FTypes> ExFuture<Runnable,ReturnTuple> bulk_debug(ExFuture<Runnable,TArg> fut, FTypes... functions)
    {
        //I will remove ReturnTuple from template when be able to atuoatically deduce from FTypes
        ExFuture<Runnable,ReturnTuple> result(fut.ex);        

        fut.subscribeCallback(            
            //std::function<::core::ECallbackResult( const typename core::FutureValue<TArg>&)>([ex = fut.ex,result,functions... ](const typename core::FutureValue<TArg>& input)  mutable
            std::function<::core::ECallbackResult( const typename core::FutureValue<TArg>&)>([ex = fut.ex,result,fs = std::make_tuple(std::forward<FTypes>(functions)... )](const typename core::FutureValue<TArg>& input) mutable
            {
                ReturnTuple* res = new ReturnTuple;
                ::parallelism::Barrier barrier(std::tuple_size_v<decltype(fs)>);
                 //_private::_invoke(ex,barrier,input,std::forward<FTypes>(fs)...);
                //_private::_invoke(ex,barrier,input,std::get<FTypes>(fs)...);
                _private::_invoke_debug<0>(ex,barrier,res,input,std::forward<FTypes>(std::get<FTypes>(fs))...);
                barrier.subscribeCallback(
                    std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([result,res](const ::parallelism::BarrierData& ) mutable
                    {
                        result.setValue(std::move(*res));
                        delete res;
                        return ::core::ECallbackResult::UNSUBSCRIBE; 
                    }));
                    
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            }));       
        
        return result;
    }
    */
    /**
     * @brief execute given functions possibly parallel (it's up to the executor to be able to do id)
     * @return en new Future whose value is moved from input future
     * */
/*
    hacer version generica
    template <class TArg,class ...FTypes> ExFuture<Runnable,TArg> bulk(ExFuture<Runnable,TArg> fut, FTypes... functions)
    {
        ExFuture<Runnable,TArg> result(fut.ex);
        typedef typename ExFuture<Runnable,TArg>::ValueType  ValueType;
        fut.subscribeCallback(            
            std::function<::core::ECallbackResult( ValueType&)>([fut,result,fs = std::make_tuple(std::forward<FTypes>(functions)... )](ValueType& input)  mutable
            {
                ::parallelism::Barrier barrier(std::tuple_size_v<decltype(fs)>);
                _private::_invoke(fut,barrier,std::forward<FTypes>(std::get<FTypes>(fs))...);
                barrier.subscribeCallback(
                    std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([fut,result](const ::parallelism::BarrierData& ) mutable
                    {
                        // @todo tengo que meditar muy bien esto
                        result.assign(std::move(fut.getValue()));
                        return ::core::ECallbackResult::UNSUBSCRIBE; 
                    }));
                    
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            }));       
        
        return result;
    }
    */
    
    typedef Executor<Runnable> RunnableExecutor; //alias
}