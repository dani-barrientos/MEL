#pragma once
//#include <execution/Continuation.h>
#include <execution/CommonDefs.h>
#include <type_traits>
/**
 * @brief High level execution utilities
 * @details This namespace contains class, functions..to give a consisten execution interface independet of the underliying execution system
 */
namespace execution
{        
    enum  EErrorCodes
    {
        //@todo completar y dar sentido
        ERROR_UNKNOWN,
        ERROR_EXCEPTION
    };
    template <typename ExecutorAgent,typename ResultType,typename ErrorType = ::core::ErrorInfo> class ExFuture;// predeclaration
    template <class ExecutorAgent> class Executor    
    {
        //mandatory interface to imlement in specializations
        template <class TRet,class F,class ErrorType = ::core::ErrorInfo> void launch( F&& f,ExFuture<ExecutorAgent,TRet,ErrorType> output) const;
        template <class TRet,class TArg,class F,class ErrorType = ::core::ErrorInfo> void launch( F&& f,TArg&& arg,ExFuture<ExecutorAgent,TRet,ErrorType> output) const;
        template <class I, class F>	 ::parallelism::Barrier loop(I&& begin, I&& end, F&& functor, int increment);
    };
    /**
     * @brief Extension of core::Future to apply to executors
     *      
     */
    template <typename ExecutorAgent,typename ResultType,typename ErrorType>
	class ExFuture : public Future<ResultType,ErrorType>
    {
        public:
            ExFuture(const ExFuture& ob):Future<ResultType,ErrorType>(ob),ex(ob.ex){}
            ExFuture(ExFuture&& ob):Future<ResultType,ErrorType>(std::move(ob)),ex(std::move(ob.ex)){}
            ExFuture(Executor<ExecutorAgent> aEx):ex(aEx){}            
            ExFuture(Executor<ExecutorAgent> aEx,ResultType& val):Future<ResultType,ErrorType>(val),ex(aEx){}
            ExFuture(Executor<ExecutorAgent> aEx,ResultType&& val):Future<ResultType,ErrorType>(std::move(val)),ex(aEx){}
		
            ExFuture& operator= ( const ExFuture& f )
            {
                Future<ResultType,ErrorType>::operator=( f );
                ex = f.ex;
                return *this;
            };
            ExFuture& operator= ( ExFuture&& f )
            {
                Future<ResultType,ErrorType>::operator=( std::move(f));
                ex = std::move(f.ex);
                return *this;
            };
            Executor<ExecutorAgent> ex;
		
    };
    //for reference type
    template <typename ExecutorAgent,typename ResultType,typename ErrorType>
	class ExFuture<ExecutorAgent,ResultType&,ErrorType> : public Future<ResultType&,ErrorType>
    {
        public:
            ExFuture(const ExFuture& ob):Future<ResultType&,ErrorType>(ob),ex(ob.ex){}
            ExFuture(ExFuture&& ob):Future<ResultType&,ErrorType>(std::move(ob)),ex(std::move(ob.ex)){}
            ExFuture(Executor<ExecutorAgent> aEx):ex(aEx){}            
            ExFuture(Executor<ExecutorAgent> aEx,ResultType& val):Future<ResultType&,ErrorType>(val),ex(aEx){}
		
            ExFuture& operator= ( const ExFuture& f )
            {
                Future<ResultType&,ErrorType>::operator=( f );
                ex = f.ex;
                return *this;
            };
            ExFuture& operator= ( ExFuture&& f )
            {
                Future<ResultType&,ErrorType>::operator=( std::move(f));
                ex = std::move(f.ex);
                return *this;
            };
            Executor<ExecutorAgent> ex;
		
    };
    //specialization for void
    template <typename ExecutorAgent,typename ErrorType>
	class ExFuture<ExecutorAgent,void,ErrorType> : public Future<void,ErrorType>
    {
        public:
            ExFuture(const ExFuture& ob):Future<void,ErrorType>(ob),ex(ob.ex){}
            ExFuture(ExFuture&& ob):Future<void,ErrorType>(std::move(ob)),ex(std::move(ob.ex)){}
            ExFuture(Executor<ExecutorAgent> aEx):ex(aEx){}            
            ExFuture(Executor<ExecutorAgent> aEx,int dummy):Future<void,ErrorType>(dummy),ex(aEx)
            {}            
            ExFuture& operator= ( const ExFuture& f )
            {
                Future<void,ErrorType>::operator=( f );
                ex = f.ex;
                return *this;
            };
            ExFuture& operator= ( ExFuture&& f )
            {
                Future<void,ErrorType>::operator=( std::move(f));
                ex = std::move(f.ex);
                return *this;
            };

            Executor<ExecutorAgent> ex;
		
    };
    
    template <class ErrorType = ::core::ErrorInfo,class F,class ExecutorAgent> ExFuture<ExecutorAgent,std::invoke_result_t<F>,ErrorType> launch( Executor<ExecutorAgent> ex,F&& f)
    {        
        typedef std::invoke_result_t<F> TRet;
        ExFuture<ExecutorAgent,TRet,ErrorType> result(ex);
        try
        {
            ex. template launch<TRet>(std::forward<F>(f),result);
        }catch( std::exception& e)
        {
            result.setError( ErrorType(EErrorCodes::ERROR_EXCEPTION,e.what()) );	
        }catch(...)
        {
            result.setError( ErrorType(EErrorCodes::ERROR_UNKNOWN,"Unknown exception on execution::launch") );	
        }
        return result;
    }
    template <class ErrorType = ::core::ErrorInfo,class TArg,class F,class ExecutorAgent> ExFuture<ExecutorAgent,std::invoke_result_t<F,TArg>,ErrorType> launch( Executor<ExecutorAgent> ex,F&& f,TArg&& arg)
    {
        /*
        @todo I need to mature this idea. It's not so transparent to add reference check
        but same rules as for "inmediate" should be followed
        static_assert( !std::is_lvalue_reference<TArg>::value ||
            std::is_const< typename std::remove_reference<TArg>::type>::value,"execution::launch. Use std::ref() to pass argument as reference");
            */
        typedef std::invoke_result_t<F,TArg> TRet;

        ExFuture<ExecutorAgent,TRet,ErrorType> result(ex);
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
    template <class ErrorType = ::core::ErrorInfo,class ExecutorAgent> ExFuture<ExecutorAgent,void,ErrorType> start( Executor<ExecutorAgent> ex)
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
    template <class ExecutorAgent,class TArg,class TRet,class ErrorType = ::core::ErrorInfo> 
        ExFuture<ExecutorAgent,typename std::remove_cv<typename std::remove_reference<TRet>::type>::type,ErrorType> inmediate( ExFuture<ExecutorAgent,TArg,ErrorType> fut,TRet&& arg)
    {
        static_assert( !std::is_lvalue_reference<TRet>::value ||
                        std::is_const< typename std::remove_reference<TRet>::type>::value,"execution::inmediate. Use std::ref() to pass argument as reference");
        using NewType = typename std::remove_cv<typename std::remove_reference<TRet>::type>::type;
        typedef typename ExFuture<ExecutorAgent,TArg,ErrorType>::ValueType  ValueType;
        ExFuture<ExecutorAgent,NewType,ErrorType> result(fut.ex);
        fut.subscribeCallback(
            std::function<::core::ECallbackResult( ValueType&)>([fut,result,arg = std::forward<TRet>(arg)](  ValueType& input) mutable
            {                                 
                 //launch tasks as response for callback for two reasons: manage the case when Future is already available when checked, so callback is trigered
                 //on calling thread and to decouple tasks and no staturate execution resoruce and independence tasks
                 //hasta aquí las copias tienen sentido
                if ( input.isValid() )
                {                    
                    // using NewType = typename std::remove_cv<typename std::remove_reference<TRet>::type>::type;
                    // fut.ex. template launch<NewType>([]( TRet&& arg) mutable
                    // {
                    //    return arg;
                    // },std::forward<TRet>(arg),result);                                                           
                    //otra forma
                    launch(fut.ex,[result,arg = std::forward<TRet>(arg)]() mutable
                    {
                        result.setValue(std::forward<TRet>(arg));                        
                    });
                }
                else
                {
                    //set error as task in executor
                    launch(fut.ex,[result,err = input.error()]( ) mutable
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
     */
    template <class F,class TArg,class ExecutorAgent,class ErrorType = ::core::ErrorInfo> ExFuture<ExecutorAgent,std::invoke_result_t<F,TArg&>,ErrorType> 
        next(ExFuture<ExecutorAgent,TArg,ErrorType> source, F&& f)
    {                
        typedef typename ExFuture<ExecutorAgent,TArg,ErrorType>::ValueType  ValueType;
        typedef std::invoke_result_t<F,TArg&> TRet;
        ExFuture<ExecutorAgent,TRet,ErrorType> result(source.ex);
        source.subscribeCallback(
            //need to bind de source future to not get lost and input pointing to unknown place                
            std::function<::core::ECallbackResult( ValueType&)>([source,f = std::forward<F>(f),result](  ValueType& input) mutable
            {       

                if ( input.isValid() )
                {                                       
                    source.ex. template launch<TRet>([f=std::forward<F>(f)](ExFuture<ExecutorAgent,TArg,ErrorType> arg) mutable
                    {                                          
                        return f(arg.getValue().value());                         
                    },source,result);
                }
                else
                {
                    //set error as task in executor
                    launch(source.ex,[result,err = input.error()]( ) mutable
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
    template <class F,class ExecutorAgent,class ErrorType = ::core::ErrorInfo> ExFuture<ExecutorAgent,std::invoke_result_t<F>,ErrorType> 
        next(ExFuture<ExecutorAgent,void,ErrorType> source, F&& f)
    {                
        typedef typename ExFuture<ExecutorAgent,void,ErrorType>::ValueType  ValueType;
        typedef std::invoke_result_t<F> TRet;
        ExFuture<ExecutorAgent,TRet,ErrorType> result(source.ex);
        source.subscribeCallback(
            //need to bind de source future to not get lost and input pointing to unknown place                
            std::function<::core::ECallbackResult( ValueType&)>([source,f = std::forward<F>(f),result](  ValueType& input) mutable
            {       

                if ( input.isValid() )
                {                                       
                    source.ex. template launch<TRet>([f=std::forward<F>(f)](ExFuture<ExecutorAgent,void,ErrorType> arg)->TRet
                    {                                          
                        return f();                         
                    },source,result);
                }
                else
                {
                    //set error as task in executor
                    launch(source.ex,[result,err = input.error()]( ) mutable
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
    template <class NewExecutorAgent,class OldExecutorAgent,class TRet,class ErrorType = ::core::ErrorInfo> ExFuture<NewExecutorAgent,TRet> transfer(ExFuture<OldExecutorAgent,TRet,ErrorType> fut,Executor<NewExecutorAgent> newAgent)
    {
        ExFuture<NewExecutorAgent,TRet,ErrorType> result(newAgent);
        typedef typename ExFuture<OldExecutorAgent,TRet,ErrorType>::ValueType  ValueType;
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
     * @return ExFuture<ExecutorAgent,TArg> 
     */
    template <class ExecutorAgent,class TArg, class I, class F,class ErrorType = ::core::ErrorInfo>	 ExFuture<ExecutorAgent,TArg,ErrorType> loop(ExFuture<ExecutorAgent,TArg,ErrorType> source,I&& begin, I&& end, F&& functor, int increment = 1)
    {
        ExFuture<ExecutorAgent,TArg,ErrorType> result(source.ex);
        typedef typename ExFuture<ExecutorAgent,TArg,ErrorType>::ValueType  ValueType;
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
                            f(idx,source.getValue().value());
                        }
                        , increment);
                        barrier.subscribeCallback(
                            std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([result,source](const ::parallelism::BarrierData& ) mutable
                            {
                                //result.assign(std::move(source.getValue()));  no puedo hacer el move porque invalido el future original
                                result.assign(source.getValue());
                                return ::core::ECallbackResult::UNSUBSCRIBE; 
                            }));
                    }else
                    {
                        //set error as task in executor
                        launch(source.ex,[result,err = input.error()]( ) mutable
                        {
                            result.setError(std::move(err));
                        });
                    }
                }
                catch(std::runtime_error& err)
                {
                    result.setError(ErrorType(EErrorCodes::ERROR_EXCEPTION,err.what()));
                }
                catch(...) //@todo improve error detection
                {
                    result.setError(ErrorType(EErrorCodes::ERROR_UNKNOWN,"ExecutorAgent::loop. Unknown error"));
                }
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            }));
        return result;
    }
    //void argument overload
    template <class ExecutorAgent,class I, class F,class ErrorType = ::core::ErrorInfo>	 ExFuture<ExecutorAgent,void,ErrorType> loop(ExFuture<ExecutorAgent,void,ErrorType> source,I&& begin, I&& end, F&& functor, int increment = 1)
    {
        ExFuture<ExecutorAgent,void,ErrorType> result(source.ex);
        typedef typename ExFuture<ExecutorAgent,void,ErrorType>::ValueType  ValueType;
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
                        launch(source.ex,[result,err = input.error()]( ) mutable
                        {
                            result.setError(std::move(err));
                        });
                    }
                }
                catch(std::runtime_error& err)
                {
                    result.setError(ErrorType(EErrorCodes::ERROR_EXCEPTION,err.what()));
                }
                catch(...) //@todo improve error detection
                {
                    result.setError(ErrorType(EErrorCodes::ERROR_UNKNOWN,"ExecutorAgent::loop. Unknown error"));
                }
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            }));
        return result;
    }
    template <class ExecutorAgent,class TArg,class ...FTypes,class ErrorType = ::core::ErrorInfo> ExFuture<ExecutorAgent,TArg,ErrorType> bulk(ExFuture<ExecutorAgent,TArg,ErrorType> source, FTypes&&... functions)
    {
        ExFuture<ExecutorAgent,TArg,ErrorType> result(source.ex);
        typedef typename ExFuture<ExecutorAgent,TArg,ErrorType>::ValueType  ValueType;
        source.subscribeCallback(            
            std::function<::core::ECallbackResult( ValueType&)>([source,result,fs = std::make_tuple(std::forward<FTypes>(functions)... )](ValueType& input)  mutable
            {
                if ( input.isValid() )
                {
                    auto barrier  = source.ex.bulk(source,std::forward<FTypes>(std::get<FTypes>(fs))...);
                    barrier.subscribeCallback(
                        std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([source,result](const ::parallelism::BarrierData& ) mutable
                        {                        
                            //result.assign(std::move(source.getValue())); el move es incorrecto!!! nada garantiza que no quiera el future fuera
                            result.assign(source.getValue());
                            return ::core::ECallbackResult::UNSUBSCRIBE; 
                        }));
                }else
                {
                     //set error as task in executor
                    launch(source.ex,[result,err = input.error()]( ) mutable
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
     * @brief capture previous error, if any, and execute the function
     * this function works similar to next, but receiving an Error as the parameter and must return
     * same type as input future is
     * no error was raised in previous works of the chain, the functions is not executed
     */    
    template <class F,class TArg,class ExecutorAgent,class ErrorType = ::core::ErrorInfo> ExFuture<ExecutorAgent,TArg,ErrorType> 
        getError(ExFuture<ExecutorAgent,TArg,ErrorType> source, F&& f)
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
    namespace _private
    {
        template <class TRet> struct ApplyInmediate
        {
             template <class T>
             ApplyInmediate(T&& a):arg(std::forward<T>(a)){}
            TRet arg;
            template <class TArg,class ExecutorAgent,class ErrorType = ::core::ErrorInfo> auto operator()(ExFuture<ExecutorAgent,TArg,ErrorType> fut)
            {
                return execution::inmediate(fut,std::forward<TRet>(arg));
            }
        };
        template <class NewExecutionAgent> struct ApplyTransfer
        {
            ApplyTransfer(Executor<NewExecutionAgent>&& a):newAgent(std::forward<Executor<NewExecutionAgent>>(a)){}
            ApplyTransfer(const Executor<NewExecutionAgent>& a):newAgent(a){}
            Executor<NewExecutionAgent> newAgent;
            template <class TRet,class OldExecutionAgent,class ErrorType = ::core::ErrorInfo> ExFuture<NewExecutionAgent,TRet,ErrorType> operator()(ExFuture<OldExecutionAgent,TRet,ErrorType> fut)
            {
                return execution::transfer(fut,newAgent);
            }
        };
        template <class F> struct ApplyNext
        {
            ApplyNext(F&& f):mFunc(std::forward<F>(f)){}
            ApplyNext(const F& f):mFunc(f){}
            F mFunc;
            template <class TArg,class ExecutorAgent,class ErrorType = ::core::ErrorInfo> auto operator()(ExFuture<ExecutorAgent,TArg,ErrorType> inputFut)
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
            template <class TArg,class ExecutorAgent,class ErrorType = ::core::ErrorInfo> auto operator()(ExFuture<ExecutorAgent,TArg,ErrorType> inputFut)
            {
                return execution::loop(inputFut,begin,end,std::forward<F>(mFunc));
            }
        };
        
        template <class ...FTypes> struct ApplyBulk
        {
            ApplyBulk(FTypes&&... fs):mFuncs(std::forward<FTypes>(fs)...){}
            ApplyBulk(const FTypes&... fs):mFuncs(fs...){}           
            std::tuple<FTypes...> mFuncs;
            template <class TArg,class ExecutorAgent,class ErrorType = ::core::ErrorInfo> auto operator()(ExFuture<ExecutorAgent,TArg,ErrorType> inputFut)
            {
                return execution::bulk(inputFut,std::forward<FTypes>(std::get<FTypes>(mFuncs))...);
            }
        };
        template <class F> struct ApplyError
        {
            ApplyError(F&& f):mFunc(std::forward<F>(f)){}
            ApplyError(const F& f):mFunc(f){}
            F mFunc;
            template <class TArg,class ExecutorAgent,class ErrorType = ::core::ErrorInfo> auto operator()(ExFuture<ExecutorAgent,TArg,ErrorType> inputFut)
            {
                return ::execution::getError(inputFut,std::forward<F>(mFunc));
            }
        };
        
    }
    //@brief version for use with operator |
    template <class TRet> _private::ApplyInmediate<TRet> inmediate( TRet&& arg)
    {
        return _private::ApplyInmediate<TRet>(std::forward<TRet>(arg));
    }
    //@brief version for use with operator |
    template <class NewExecutionAgent> _private::ApplyTransfer<NewExecutionAgent> transfer(Executor<NewExecutionAgent> newAgent)
    {
        return _private::ApplyTransfer<NewExecutionAgent>(newAgent);
    }
    
    //@brief version for use with operator |
    template <class F> _private::ApplyNext<F> next(F&& f)
    {
        return _private::ApplyNext<F>(std::forward<F>(f));
    }
    template <class I,class F> _private::ApplyLoop<I,F> loop(I&& begin, I&& end, F&& functor, int increment = 1)
    {
        return _private::ApplyLoop<I,F>(begin,end,std::forward<F>(functor),increment);
    }
    template <class ...FTypes> _private::ApplyBulk<FTypes...> bulk(FTypes&&... functions)
    {
        return _private::ApplyBulk<FTypes...>(std::forward<FTypes>(functions)...);
    }
     template <class F> _private::ApplyError<F> getError(F&& f)
    {
        return _private::ApplyError<F>(std::forward<F>(f));
    }
    /**
     * @brief overload operator | for chaining
    */
    template <class ExecutorAgent,class TRet1,class U,class ErrorType = ::core::ErrorInfo> auto operator | (ExFuture<ExecutorAgent,TRet1,ErrorType> input,U&& u)
    {
        return u(input);

    }            

}