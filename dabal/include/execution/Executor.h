#pragma once
//#include <execution/Continuation.h>
#include <execution/CommonDefs.h>
#include <type_traits>
#include <string>
#include <stdexcept>
#include <execution/ExFuture.h>
/**
 * @brief High level execution utilities
 * @details This namespace contains class, functions..to give a consistent execution interface independent of the underliying execution system
 */
namespace execution
{       
   // template <typename ExecutorAgent,typename ResultType> class ExFuture;// predeclaration
    template <class ExecutorAgent> class Executor    
    {
        //mandatory interface to imlement in specializations
        template <class TRet,class F> void launch( F&& f,ExFuture<ExecutorAgent,TRet> output) const;
        template <class TRet,class TArg,class F> void launch( F&& f,TArg&& arg,ExFuture<ExecutorAgent,TRet> output) const;
        template <class I, class F>	 ::parallelism::Barrier loop(I&& begin, I&& end, F&& functor, int increment);
        template <class TArg,class ...FTypes> ::parallelism::Barrier parallel(ExFuture<ExecutorAgent,TArg> fut, FTypes&&... functions);
        template <class ReturnTuple,class TArg,class ...FTypes> ::parallelism::Barrier parallel_convert(ExFuture<ExecutorAgent,TArg> fut,ReturnTuple& result, FTypes&&... functions);
    };
  		
    // };
    /**
     * @brief Launch given functor in given executor
     * @return ExFuture with return type of function
     */
    template <class F,class ExecutorAgent> ExFuture<ExecutorAgent,std::invoke_result_t<F>> launch( Executor<ExecutorAgent> ex,F&& f)
    {        
        typedef std::invoke_result_t<F> TRet;
        ExFuture<ExecutorAgent,TRet> result(ex);
        ex. template launch<TRet>(std::forward<F>(f),result);

        return result;
    }
    /**
     * @brief Launch given functor in given executor, passing it input parameter
     * @return ExFuture with return type of function
     */
    template <class TArg,class F,class ExecutorAgent> ExFuture<ExecutorAgent,std::invoke_result_t<F,TArg>> launch( Executor<ExecutorAgent> ex,F&& f,TArg&& arg)
    {
        /*
        @todo I need to mature this idea. It's not so transparent to add reference check
        but same rules as for "inmediate" should be followed
        static_assert( !std::is_lvalue_reference<TArg>::value ||
            std::is_const< typename std::remove_reference<TArg>::type>::value,"execution::launch. Use std::ref() to pass argument as reference");
            */
        typedef std::invoke_result_t<F,TArg> TRet;

        ExFuture<ExecutorAgent,TRet> result(ex);
        ex. template launch<TRet>(std::forward<F>(f),std::forward<TArg>(arg),result);                
        return result;
    }
    /**
     * @brief Start a chain of execution in given executor.     
     * 
     * @tparam ExecutorAgent 
     * @param ex 
     * @return ExFuture<ExecutorAgent,void> 
     */
    template <class ExecutorAgent> ExFuture<ExecutorAgent,void> start( Executor<ExecutorAgent> ex)
    {
        return launch(ex,[]{});
        //it's wrong, need to be executed in executor's context        
        // ExFuture<ExecutorAgent,void> result(ex,0);  //using an int to fool and construct it already as valid
        // return result;
    }   
    /**
     * @brief Produces an inmediate value in the context of the given ExFuture executor as a response to input fut completion
     * If input fut has error, the this error is forwarded to inmediate result
     */
    template <class ExecutorAgent,class TArg,class TRet> 
        ExFuture<ExecutorAgent,typename std::remove_cv<typename std::remove_reference<TRet>::type>::type> inmediate( ExFuture<ExecutorAgent,TArg> fut,TRet&& arg)
    {
        static_assert( !std::is_lvalue_reference<TRet>::value ||
                        std::is_const< typename std::remove_reference<TRet>::type>::value,"execution::inmediate. Use std::ref() to pass argument as reference");
        using NewType = typename std::remove_cv<typename std::remove_reference<TRet>::type>::type;
        typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
        ExFuture<ExecutorAgent,NewType> result(fut.ex);
        fut.subscribeCallback(
            std::function<::core::ECallbackResult( ValueType&)>([fut,result,arg = std::forward<TRet>(arg)](  ValueType& input) mutable
            {                                 
                 //launch tasks as response for callback for two reasons: manage the case when Future is already available when checked, so callback is trigered
                 //on calling thread and to decouple tasks and no staturate execution resoruce and independence tasks
                 //hasta aquí las copias tienen sentido
                if ( input.isValid() )
                {                                    
                    launch(fut.ex,[result,arg = std::forward<TRet>(arg)]() mutable noexcept
                    {
                        result.setValue(std::forward<TRet>(arg));                        
                    });
                }
                else
                {
                    //set error as task in executor
                    launch(fut.ex,[result,err = std::move(input.error())]( ) mutable noexcept
                    {
                       result.setError(std::move(err));
                    });
                }
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            })
        );
        return result;  
    }

    /**
     * @brief Attach a functor to execute when input fut is complete
     * Given functor will be executed inf the input ExFuture executor. 
     * input parameter is always pass as reference
     * @return An ExFuture with type given by functor result.
     */
    template <class F,class TArg,class ExecutorAgent> ExFuture<ExecutorAgent,std::invoke_result_t<F,TArg&>> 
        next(ExFuture<ExecutorAgent,TArg> source, F&& f)
    {                
        typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
        typedef std::invoke_result_t<F,TArg&> TRet;
        ExFuture<ExecutorAgent,TRet> result(source.ex);
        source.subscribeCallback(
            //need to bind de source future to not get lost and input pointing to unknown place                
            std::function<::core::ECallbackResult( ValueType&)>([source,f = std::forward<F>(f),result](  ValueType& input) mutable
            {       

                if ( input.isValid() )
                {    
                    source.ex. template launch<TRet>([f=std::forward<F>(f)](ExFuture<ExecutorAgent,TArg>& arg) mutable noexcept(std::is_nothrow_invocable<F,TArg&>::value)->TRet 
                    {                                          
                        return f(arg.getValue().value());                         
                    },source,result);            
                }
                else
                {
                    //set error as task in executor
                    launch(source.ex,[result,err = std::move(input.error())]( ) mutable noexcept
                    {
                       result.setError(std::move(err));
                    });
                }
                  
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            })
        );
        return result;
    }
    //overload for void arg
    template <class F,class ExecutorAgent> ExFuture<ExecutorAgent,std::invoke_result_t<F>> 
        next(ExFuture<ExecutorAgent,void> source, F&& f)
    {                
        typedef typename ExFuture<ExecutorAgent,void>::ValueType  ValueType;
        typedef std::invoke_result_t<F> TRet;
        ExFuture<ExecutorAgent,TRet> result(source.ex);
        source.subscribeCallback(
            //need to bind de source future to not get lost and input pointing to unknown place                
            std::function<::core::ECallbackResult( ValueType&)>([source,f = std::forward<F>(f),result](  ValueType& input) mutable
            {       

                if ( input.isValid() )
                {                                       
                    if constexpr (noexcept(f()))
                    {
                        source.ex. template launch<TRet>([f=std::forward<F>(f)](ExFuture<ExecutorAgent,void>& arg) noexcept->TRet
                        {                                          
                            return f();                         
                        },source,result);
                    }else
                    {
                        source.ex. template launch<TRet>([f=std::forward<F>(f)](ExFuture<ExecutorAgent,void>& arg)->TRet
                        {                                          
                            return f();                         
                        },source,result);
                    }
                }
                else
                {
                    //set error as task in executor
                    launch(source.ex,[result,err = std::move(input.error())]( ) mutable noexcept
                    {
                       result.setError(std::move(err));
                    });
                }
                  
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            })
        );
        return result;
    }
       
    /**
     * @brief Transfer given ExFuture to a different executor 
     * This way, continuations can be chained but executed in diferent executors
     * @code {.cpp}
     * 
     *  Create two different threads
        auto th1 = ThreadRunnable::create(true);	
		auto th2 = ThreadRunnable::create(true);
        //create executor from one of the threads
        execution::Executor<Runnable> ex(th1);		
		ex.setOpts({true,false,false});	
        //launch chain of functions starting in executor ex
      	auto fut = execution::next(execution::schedule(ex),
            [](const auto& v)->int
            {					
                text::info("Current Runnable {}",static_cast<void*>(ThreadRunnable::getCurrentRunnable()));
                text::info("Launch waiting");
                if ( ::tasking::Process::wait(5000) != tasking::Process::ESwitchResult::ESWITCH_KILL )
                {
                    //throw std::runtime_error("Error1");
                    text::info("Launch done");
                }else
                    text::info("Launch killed");
                return 4;
            }
        );
        //create another executor using the other thread..
		execution::Executor<Runnable> ex2(th2);
		//..and tranfer current execution chain to new executor
		fut = execution::transfer(fut,ex2);
        //..so chaining, continues from this new executor
		auto fut2 = execution::next(fut,[](const auto& v)
            {
                text::info("Current Runnable {}",static_cast<void*>(ThreadRunnable::getCurrentRunnable()));
                if (v.isValid())
                {
                    text::info("Next done: {}",v.value());		
                }else					
                    text::error("Next error: {}",v.error().errorMsg);		
            }
        );						
		::core::waitForFutureThread(fut2); //wait for result from current thread	
     * @endcode
     */
    template <class NewExecutorAgent,class OldExecutorAgent,class TRet> ExFuture<NewExecutorAgent,TRet> transfer(ExFuture<OldExecutorAgent,TRet> fut,Executor<NewExecutorAgent> newAgent)
    {
        ExFuture<NewExecutorAgent,TRet> result(newAgent);
        typedef typename ExFuture<OldExecutorAgent,TRet>::ValueType  ValueType;
        fut.subscribeCallback(
            std::function<::core::ECallbackResult( ValueType&)>([result](ValueType& input) mutable
            {
                result.assign(input);
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            })
        );
        return result;
    }   
    /**
     * @brief parallel (possibly, depending on executor capabilities) loop
     * @note concrete executor must provide a member loop function with neccesary interface
     * @return A ExFuture with same value as input future, whose content IS MOVED
     */
    template <class ExecutorAgent,class TArg, class I, class F>	 ExFuture<ExecutorAgent,TArg> loop(ExFuture<ExecutorAgent,TArg> source,I&& begin, I&& end, F&& functor, int increment = 1)
    {
        ExFuture<ExecutorAgent,TArg> result(source.ex);
        typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
        source.subscribeCallback(
            std::function<::core::ECallbackResult( ValueType&)>([source,functor = std::forward<F>(functor),result,begin = std::forward<I>(begin),end = std::forward<I>(end),increment](ValueType& input)  mutable
            {
                try
                {   
                    if ( input.isValid() )
                    {
                        std::exception_ptr* except = new std::exception_ptr(nullptr);
                        ::parallelism::Barrier barrier;
                        if constexpr (std::is_nothrow_invocable<F,I,TArg&>::value)
                        {
                            barrier  = source.ex.loop(std::forward<I>(begin), std::forward<I>(end),
                            [f = std::forward<F>(functor),source](I idx) mutable noexcept
                            {
                                //@todo arreglar el loop para que reciba I&&
                                //f(std::forward<I>(idx),fut.getValue());

                                f(idx,source.getValue().value());                         
                            }
                            , increment);
                        }else
                        {
                            barrier  = source.ex.loop(std::forward<I>(begin), std::forward<I>(end),
                            [f = std::forward<F>(functor),source,except](I idx) mutable
                            {
                                try
                                {
                                    f(idx,source.getValue().value());                         
                                }catch(...)
                                {
                                    if (!*except)
                                        *except = std::current_exception();
                                }
                            }
                            , increment);
                        }
                        barrier.subscribeCallback(
                            std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([result,source,except](const ::parallelism::BarrierData& ) mutable
                            {                
                                if ( *except ) //any exception?
                                    result.setError(*except);
                                else
                                    result.assign(std::move(source.getValue())); //@todo it's not correct, but necesary to avoid a lot of copies. I left this way until solved in the root. Really is not very worrying
                                    //result.assign(source.getValue());
                                delete except;
                                return ::core::ECallbackResult::UNSUBSCRIBE; 
                            }));
                    }else
                    {
                        //set error as task in executor
                        launch(source.ex,[result,err = std::move(input.error())]( ) mutable noexcept
                        {
                            result.setError(std::move(err));
                        });
                    }
                }
                catch(...) 
                {
                    result.setError( std::current_exception() );	
                }
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            }));
        return result;
    }
    //void argument overload
    template <class ExecutorAgent,class I, class F>	 ExFuture<ExecutorAgent,void> loop(ExFuture<ExecutorAgent,void> source,I&& begin, I&& end, F&& functor, int increment = 1)
    {
        ExFuture<ExecutorAgent,void> result(source.ex);
        typedef typename ExFuture<ExecutorAgent,void>::ValueType  ValueType;
        source.subscribeCallback(
            std::function<::core::ECallbackResult( ValueType&)>([source,functor = std::forward<F>(functor),result,begin = std::forward<I>(begin),end = std::forward<I>(end),increment](ValueType& input)  mutable
            {
                try
                {   
                    if ( input.isValid() )
                    {
                        auto barrier  = source.ex.loop(std::forward<I>(begin), std::forward<I>(end),
                        [f = std::forward<F>(functor),source](I idx) mutable
                        {
                            //@todo arreglar el loop para que reciba I&&
                            //f(std::forward<I>(idx),fut.getValue());
                            f(idx);
                        }
                        , increment);
                        barrier.subscribeCallback(
                            std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([result,source](const ::parallelism::BarrierData& ) mutable
                            {
                                result.assign(std::move(source.getValue()));
                                return ::core::ECallbackResult::UNSUBSCRIBE; 
                            }));
                    }else
                    {
                        //set error as task in executor
                        launch(source.ex,[result,err = std::move(input.error())]( )  mutable noexcept
                        {
                            result.setError(std::move(err));
                        });
                    }
                }
               
                catch(...) 
                {
                    result.setError( std::current_exception() );	
                }
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            }));
        return result;
    }
    /**
     * @brief Execute given functions in a (possibly, depending con concrete executor) parallel way
     * If an exception is thrown in any of the callables, and noexcept is no specified, the
     * value of the first exception thrown is set as error in the resulting future, so forwarding the error to the next element (if any) of the chain
     @return A ExFuture with same value as input future, whose content IS MOVED
     */
    template <class ExecutorAgent,class TArg,class ...FTypes> ExFuture<ExecutorAgent,TArg> parallel(ExFuture<ExecutorAgent,TArg> source, FTypes&&... functions)
    {
        ExFuture<ExecutorAgent,TArg> result(source.ex);
        typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
        source.subscribeCallback(            
            std::function<::core::ECallbackResult( ValueType&)>([source,result,fs = std::make_tuple(std::forward<FTypes>(functions)... )](ValueType& input)  mutable
            {
                if ( input.isValid() )
                {
                    std::exception_ptr* except = new std::exception_ptr(nullptr);
                    auto barrier  = source.ex.parallel(source,*except,std::forward<FTypes>(std::get<FTypes>(fs))...);
                    barrier.subscribeCallback(
                        std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([source,result,except](const ::parallelism::BarrierData& ) mutable
                        {                 
                            if ( *except ) //any exception?
                                result.setError(*except);
                            else
                                result.assign(std::move(source.getValue()));//@todo it's not correct, but necesary to avoid a lot of copies. I left this way until solved in the root. Really is not very worrying
                            delete except;
                            return ::core::ECallbackResult::UNSUBSCRIBE; 
                        }));
                }else
                {
                     //set error as task in executor
                    launch(source.ex,[result,err = std::move(input.error())]( ) mutable noexcept
                    {
                       result.setError(std::move(err));
                    });
                }
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            }
        ));
        return result;
    }
  
    /**
     * @brief Capture previous error, if any, and execute the function
     * this function works similar to next, but receiving an std::exception_ptr as the parameter and must return
     * same type as input future is
     * no error was raised in previous works of the chain, the functions is not executed
     */    
    template <class F,class TArg,class ExecutorAgent> ExFuture<ExecutorAgent,TArg> 
        catchError(ExFuture<ExecutorAgent,TArg> source, F&& f)
    {                
        typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
        ExFuture<ExecutorAgent,TArg> result(source.ex);
        source.subscribeCallback(
            //need to bind de source future to not get lost and input pointing to unknown place                
            std::function<::core::ECallbackResult( ValueType&)>([source,f = std::forward<F>(f),result]( ValueType& input) mutable
            {       
                if ( !input.isValid() )
                {                
                    source.ex. template launch<TArg>([f=std::forward<F>(f)](ExFuture<ExecutorAgent,TArg>& arg) mutable noexcept(std::is_nothrow_invocable<F,std::exception_ptr>::value) -> TArg
                    {                                          
                        return f(arg.getValue().error());
                    },source,result);
                }else
                    result.assign(source.getValue());
                  
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            })
        );
        return result;
    }
    /**
     * @brief Capture previous error, if any, and execute the function
     * this function works similar to next, but receiving an Error as the parameter and must return
     * same type as input future is
     * no error was raised in previous works of the chain, the functions is not executed
     */    
/*
no lo tengo claro. necesito poder hacer cosas sobre el executor, pero no lo tengo en ningún lado.
mi idea es que tal vez con un algoritmo que lo capture...igual es una idiotez
igual no tiene mucho sentido y 
    template <class F,class TArg,class ExecutorAgent,class ErrorType = ::core::ErrorInfo> ExFuture<ExecutorAgent,TArg,ErrorType> 
        getExecutor(ExFuture<ExecutorAgent,TArg,ErrorType> source, F&& f)
    {                
        typedef typename ExFuture<ExecutorAgent,TArg,ErrorType>::ValueType  ValueType;
        ExFuture<ExecutorAgent,TArg,ErrorType> result(source.ex);
        source.subscribeCallback(
            //need to bind de source future to not get lost and input pointing to unknown place                
            std::function<::core::ECallbackResult( ValueType&)>([source,f = std::forward<F>(f),result]( ValueType& input) mutable
            {       
                if ( !input.isValid() )
                {                                       
                    source.ex. template launch<TArg>([f=std::forward<F>(f)](ExFuture<ExecutorAgent,TArg,ErrorType> arg)
                    {                                          
                        return f(arg.getValue().error());                         
                    },source,result);
                }else
                    result.assign(source.getValue());
                  
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            })
        );
        return result;
    }
  */  
    /**
     * @brief Same as @ref parallel but returning a tuple with the values for each functor
     * So, these functors must return a value, return void is not alloed
     */
    template <class ResultTuple, class ExecutorAgent,class TArg,class ...FTypes> ExFuture<ExecutorAgent,ResultTuple> parallel_convert(ExFuture<ExecutorAgent,TArg> source, FTypes&&... functions)
    {
        //@todo tratar de dedudir la tupla de los resultados de cada funcion
        static_assert(std::is_default_constructible<ResultTuple>::value,"All types returned by the input ExFutures must be DefaultConstructible");
        ExFuture<ExecutorAgent,ResultTuple> result(source.ex);
        typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
        source.subscribeCallback(            
            std::function<::core::ECallbackResult( ValueType&)>([source,result,fs = std::make_tuple(std::forward<FTypes>(functions)... )](ValueType& input)  mutable
            {
                if ( input.isValid() )
                {                       
                    std::exception_ptr* except = new std::exception_ptr(nullptr);
                    ResultTuple* output = new ResultTuple; //para que compile
                    auto barrier  = source.ex.parallel_convert(source,*except,*output,std::forward<FTypes>(std::get<FTypes>(fs))...);
                    barrier.subscribeCallback(
                        std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([result,output,except](const ::parallelism::BarrierData& ) mutable
                        {      
                            if ( *except ) //any exception?
                                result.setError(*except);
                            else
                                result.setValue(std::move(*output));
                            delete output;    
                            delete except;              
                            return ::core::ECallbackResult::UNSUBSCRIBE; 
                        }));
                }else
                {
                     //set error as task in executor
                    launch(source.ex,[result,err = std::move(input.error())]( ) mutable noexcept
                    {
                       result.setError(std::move(err));
                    });
                }
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            }
        ));
        return result;
    }
    ///@cond HIDDEN_SYMBOLS
    namespace _private
    {
        template <class TRet> struct ApplyInmediate
        {
             template <class T>
             ApplyInmediate(T&& a):arg(std::forward<T>(a)){}
            TRet arg;
            template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> fut)
            {
                return execution::inmediate(fut,std::forward<TRet>(arg));
            }
        };
        template <class NewExecutionAgent> struct ApplyTransfer
        {
            ApplyTransfer(Executor<NewExecutionAgent>&& a):newAgent(std::forward<Executor<NewExecutionAgent>>(a)){}
            ApplyTransfer(const Executor<NewExecutionAgent>& a):newAgent(a){}
            Executor<NewExecutionAgent> newAgent;
            template <class TRet,class OldExecutionAgent> ExFuture<NewExecutionAgent,TRet> operator()(ExFuture<OldExecutionAgent,TRet> fut)
            {
                return execution::transfer(fut,newAgent);
            }
        };
        template <class F> struct ApplyNext
        {
            ApplyNext(F&& f):mFunc(std::forward<F>(f)){}
            ApplyNext(const F& f):mFunc(f){}
            F mFunc;
            template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
            {
                return ::execution::next(inputFut,std::forward<F>(mFunc));
            }
        };
        template <class I,class F> struct ApplyLoop
        {
            ApplyLoop(I b, I e,F&& f,int inc):mFunc(std::forward<F>(f)),begin(std::move(b)),end(std::move(e)),increment(inc){}
            ApplyLoop(I b, I e,const F& f,int inc):mFunc(f),begin(std::move(b)),end(std::move(e)),increment(inc){}
            F mFunc;
            I begin;
            I end;
            int increment;
            template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
            {
                return execution::loop(inputFut,begin,end,std::forward<F>(mFunc));
            }
        };
        
        template <class ...FTypes> struct ApplyBulk
        {
            ApplyBulk(FTypes&&... fs):mFuncs(std::forward<FTypes>(fs)...){}
            ApplyBulk(const FTypes&... fs):mFuncs(fs...){}           
            std::tuple<FTypes...> mFuncs;
            template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
            {
                return execution::parallel(inputFut,std::forward<FTypes>(std::get<FTypes>(mFuncs))...);
            }
        };
        template <class F> struct ApplyError
        {
            ApplyError(F&& f):mFunc(std::forward<F>(f)){}
            ApplyError(const F& f):mFunc(f){}
            F mFunc;
            template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
            {
                return ::execution::catchError(inputFut,std::forward<F>(mFunc));
            }
        };
        template <int n,class TupleType,class FType> void _on_all(TupleType* tup,::parallelism::Barrier& barrier, FType fut)
        {
            fut.subscribeCallback(
                std::function<::core::ECallbackResult( typename FType::ValueType&)>([tup,barrier](typename FType::ValueType& input)  mutable
                {
                    std::get<n>(*tup) = std::move(input);
                    barrier.set();
                    return ::core::ECallbackResult::UNSUBSCRIBE; 
                }));
        }

        template <int n,class TupleType,class FType,class ...FTypes> void _on_all(TupleType* tup, ::parallelism::Barrier& barrier,FType fut,FTypes... rest)
        {
            _on_all<n>(tup,barrier,fut);
            _on_all<n+1>(tup,barrier,rest...);
        }
        template <int n,class SourceTuple, class TargetTuple> std::optional<std::pair<int,std::exception_ptr>> _moveValue(SourceTuple& st,TargetTuple& tt)
        {
            if constexpr (n != std::tuple_size<SourceTuple>::value)
            {
                auto& val = std::get<n>(st);
                if ( val.isValid() )
                {
                    std::get<n>(tt) = std::move(val.value());
                    return _moveValue<n+1>(st,tt);
                }else
                    return std::make_pair(n,std::move(val.error()));
            }
            return std::nullopt;
        }
        template <class ReturnTuple, class ...FTypes> struct ApplyParallelConvert
        {
            ApplyParallelConvert(FTypes&&... fs):mFuncs(std::forward<FTypes>(fs)...){}
            ApplyParallelConvert(const FTypes&... fs):mFuncs(fs...){}           
            std::tuple<FTypes...> mFuncs;
            template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
            {
                return execution::parallel_convert<ReturnTuple>(inputFut,std::forward<FTypes>(std::get<FTypes>(mFuncs))...);
            }
        };
    }
    //@brief version for use with operator |
    template <class TRet> _private::ApplyInmediate<TRet> inmediate( TRet&& arg)
    {
        return _private::ApplyInmediate<TRet>(std::forward<TRet>(arg));
    }
    /**
     * @brief Version for use with operator |     
     */
    template <class NewExecutionAgent> _private::ApplyTransfer<NewExecutionAgent> transfer(Executor<NewExecutionAgent> newAgent)
    {
        return _private::ApplyTransfer<NewExecutionAgent>(newAgent);
    }
    
    ///@brief version for use with operator |
    template <class F> _private::ApplyNext<F> next(F&& f)
    {
        return _private::ApplyNext<F>(std::forward<F>(f));
    }
    ///@brief version for use with operator |
    template <class I,class F> _private::ApplyLoop<I,F> loop(I&& begin, I&& end, F&& functor, int increment = 1)
    {
        return _private::ApplyLoop<I,F>(begin,end,std::forward<F>(functor),increment);
    }
    ///@brief version for use with operator |
    template <class ...FTypes> _private::ApplyBulk<FTypes...> parallel(FTypes&&... functions)
    {
        return _private::ApplyBulk<FTypes...>(std::forward<FTypes>(functions)...);
    }
    ///@brief version for use with operator |
    template <class F> _private::ApplyError<F> catchError(F&& f)
    {
        return _private::ApplyError<F>(std::forward<F>(f));
    }
    ///@brief version for use with operator |
    template <class ReturnTuple,class ...FTypes> _private::ApplyParallelConvert<ReturnTuple,FTypes...> parallel_convert(FTypes&&... functions)
    {
        return _private::ApplyParallelConvert<ReturnTuple,FTypes...>(std::forward<FTypes>(functions)...);
    }
    ///@endcond

    /**
     * @brief overload operator | for chaining
    */
    template <class ExecutorAgent,class TRet1,class U> auto operator | (const ExFuture<ExecutorAgent,TRet1>& input,U&& u)
    {
        return u(input);
    } 

    /**
     * @brief Excepcion thrown by on_all when some of the futures raise error
     * It contains a reference to the causing exception and the element in the tuple which caused de exception
     * 
     */
    class OnAllException : public std::runtime_error
    {
        public:
            OnAllException(int idx,std::exception_ptr source,const std::string& msg):mElementIdx(idx),mSource(source),std::runtime_error(msg){}
            OnAllException(int idx,std::exception_ptr source,std::string&& msg):mElementIdx(idx),mSource(source),std::runtime_error(std::move(msg)){}
            inline int getWrongElement() const noexcept{ return mElementIdx;}
            std::exception_ptr getCause() noexcept{ return mSource;}
        private:
            int  mElementIdx;
            std::exception_ptr mSource;
    };
    /**
     * @brief return ExFuture which will be executed, in the context of the given executor ex,
     * when all the given ExFutures are triggered. The resulting ExFuture has a std::tuple with the types if the given ExFutures
     * in the same order. So, all of them must return a value, void is not allowed
     * If any of the given input ExFuture has error, the returned one will have the same error indicating  the element who failed
     *   @throws OnAllException if any error
     */
    template <class ExecutorAgent,class ...FTypes> auto on_all(Executor<ExecutorAgent> ex,FTypes... futs)
    {                
		typedef std::tuple<typename FTypes::ValueType::Type...> ReturnType;
		static_assert(std::is_default_constructible<ReturnType>::value,"All types returned by the input ExFutures must be DefaultConstructible");
        ExFuture<ExecutorAgent,ReturnType> result(ex);
		::parallelism::Barrier barrier(sizeof...(futs));
		typedef std::tuple<typename FTypes::ValueType...>  _ttype;
		_ttype* tupleRes = new _ttype;

		barrier.subscribeCallback(
		std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([result,tupleRes](const ::parallelism::BarrierData& ) mutable
		{
			ReturnType resultVal;			
			auto r = ::execution::_private::_moveValue<0>(*tupleRes,resultVal);
			if ( r == std::nullopt)
			{
				result.setValue(std::move(resultVal));
			}else
			{
				result.setError( std::make_exception_ptr(OnAllException(r.value().first,r.value().second,"Error in tuple element"))); 
			}
			delete tupleRes;
			return ::core::ECallbackResult::UNSUBSCRIBE; 
		}));
        ::execution::_private::_on_all<0,_ttype>(tupleRes,barrier,futs...);  //la idea es pasar una barrera o lo que sea y devolver resultado al activarse
        return result;
    }           

}