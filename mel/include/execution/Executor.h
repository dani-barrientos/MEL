#pragma once
/*
 * SPDX-FileCopyrightText: 2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
#include <execution/CommonDefs.h>
#include <type_traits>
#include <string>
#include <stdexcept>
#include <execution/ExFuture.h>
#include <functional>
#include <optional>
namespace mel
{
    /**
     * @brief High level execution utilities
     * @details This namespace contains class, functions..to give a consistent execution interface independent of the underliying execution system
     * See \ref execution_system for detailed explanations and examples
     */
    namespace execution
    {       
        template <class ExecutorAgent> class Executor    
        {
            //mandatory interface to imlement in specializations
            template <class TRet,class F> void launch( F&& f,ExFuture<ExecutorAgent,TRet> output) const;
            template <class TRet,class TArg,class F> void launch( F&& f,TArg&& arg,ExFuture<ExecutorAgent,TRet> output) const;
            template <class I, class F>	 ::mel::parallelism::Barrier loop(I&& begin, I&& end, F&& functor, int increment);
            template <class TArg,class ...FTypes> ::mel::parallelism::Barrier parallel(ExFuture<ExecutorAgent,TArg> fut, FTypes&&... functions);
            template <class ReturnTuple,class TArg,class ...FTypes> ::mel::parallelism::Barrier parallel_convert(ExFuture<ExecutorAgent,TArg> fut,ReturnTuple& result, FTypes&&... functions);
        };
        /**
         * @brief Default traits for any executor
         * @details Concrete executor must set its own values when changed
         */
        template <class ExecutorType> struct ExecutorTraits
        {
            //! Support microthreading?
            enum {has_microthreading = false};  
            //! Support true parallelism?
            enum {has_parallelism = false}; //!< support true parallelism?
        };
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
            ExFuture<ExecutorAgent,NewType> result(fut.agent);
            fut.subscribeCallback(
                std::function<void( ValueType&)>([fut,result,arg = std::forward<TRet>(arg)](  ValueType& input) mutable
                {                                 
                    //launch tasks as response for callback for two reasons: manage the case when Future is already available when checked, so callback is trigered
                    //on calling thread and to decouple tasks and no saturate execution resource and independence tasks
                    if ( input.isValid() )
                    {                                    
                        launch(fut.agent,[result,arg = std::forward<TRet>(arg)]() mutable noexcept
                        {
                            result.setValue(std::forward<TRet>(arg));                        
                        });
                    }
                    else
                    {
                        //set error as task in executor
                        launch(fut.agent,[result,err = std::move(input.error())]( ) mutable noexcept
                        {
                            result.setError(std::move(err));
                        });
                    }   
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
        template <class F,class TArg,class ExecutorAgent> ExFuture<ExecutorAgent,std::invoke_result_t<F,TArg>> 
            next(ExFuture<ExecutorAgent,TArg> source, F&& f)
        {                
            static_assert( std::is_invocable<F,TArg>::value, "execution::next bad functor signature");
            typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
            typedef std::invoke_result_t<F,TArg> TRet;
            ExFuture<ExecutorAgent,TRet> result(source.agent);
            source.subscribeCallback(
                //need to bind de source future to not get lost and input pointing to unknown place                
                std::function<void( ValueType&)>([source,f = std::forward<F>(f),result](  ValueType& input) mutable
                {       
                    if ( input.isValid() )
                    {                            
                        source.agent. template launch<TRet>([f=std::forward<F>(f)](ExFuture<ExecutorAgent,TArg>& arg) mutable noexcept(std::is_nothrow_invocable<F,TArg>::value)->TRet 
                        {                                          
                            return f(arg.getValue().value());                         
                        },source,result);                                                          
                    }
                    else
                    {
                        //set error as task in executor
                        launch(source.agent,[result,err = std::move(input.error())]( ) mutable noexcept
                        {
                            result.setError(std::move(err));
                        });
                    }                                        
                })
            );
            return result;
        }
        ///@cond HIDDEN_SYMBOLS
        //overload for void arg
        template <class F,class ExecutorAgent> ExFuture<ExecutorAgent,std::invoke_result_t<F>> 
            next(ExFuture<ExecutorAgent,void> source, F&& f)
        {                            
            typedef typename ExFuture<ExecutorAgent,void>::ValueType  ValueType;
            typedef std::invoke_result_t<F> TRet;
            ExFuture<ExecutorAgent,TRet> result(source.agent);
            source.subscribeCallback(
                //need to bind de source future to not get lost and input pointing to unknown place                
                std::function<void(ValueType&)>([source,f = std::forward<F>(f),result](  ValueType& input) mutable
                {       

                    if ( input.isValid() )
                    {                                       
                        if constexpr (noexcept(f()))
                        {
                            source.agent. template launch<TRet>([f=std::forward<F>(f)](ExFuture<ExecutorAgent,void>& arg) noexcept->TRet
                            {                                          
                                return f();                         
                            },source,result);
                        }else
                        {
                            source.agent. template launch<TRet>([f=std::forward<F>(f)](ExFuture<ExecutorAgent,void>& arg)->TRet
                            {                                          
                                return f();                         
                            },source,result);
                        }
                    }
                    else
                    {
                        //set error as task in executor
                        launch(source.agent,[result,err = std::move(input.error())]( ) mutable noexcept
                        {
                        result.setError(std::move(err));
                        });
                    }
                })
            );
            return result;
        }
        ///@endcond
        /**
         * @brief Transfer given ExFuture to a different executor 
         * This way, continuations can be chained but executed in diferent executors
         */
        template <class NewExecutorAgent,class OldExecutorAgent,class TRet> ExFuture<NewExecutorAgent,TRet> transfer(ExFuture<OldExecutorAgent,TRet> source,Executor<NewExecutorAgent> newAgent)
        {
            ExFuture<NewExecutorAgent,TRet> result(newAgent);
            typedef typename ExFuture<OldExecutorAgent,TRet>::ValueType  ValueType;
            source.subscribeCallback(
                std::function<void( ValueType&)>([result,source](ValueType& input) mutable 
                {
                    //result.assign(std::move(input));
                    result.assign(source);
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
            static_assert( std::is_invocable<F,int,TArg>::value, "execution::loop bad functor signature");
            ExFuture<ExecutorAgent,TArg> result(source.agent);
            typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
            source.subscribeCallback(
                std::function<void( ValueType&)>([source,functor = std::forward<F>(functor),result,begin = std::forward<I>(begin),end = std::forward<I>(end),increment](ValueType& input)  mutable
                {
                    try
                    {   
                        if ( input.isValid() )
                        {
                            std::exception_ptr* except = new std::exception_ptr(nullptr);
                            ::mel::parallelism::Barrier barrier;
                            if constexpr (std::is_nothrow_invocable<F,I,TArg>::value)
                            {
                                barrier  = source.agent.loop(std::forward<I>(begin), std::forward<I>(end),
                                [f = std::forward<F>(functor),source](I idx) mutable noexcept
                                {
                                    //@todo arreglar el loop para que reciba I&&
                                    //f(std::forward<I>(idx),fut.getValue());

                                    f(idx,source.getValue().value());                         
                                }
                                , increment);
                            }else
                            {
                                barrier  = source.agent.loop(std::forward<I>(begin), std::forward<I>(end),
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
                                std::function<::mel::core::ECallbackResult( const ::mel::parallelism::BarrierData&)>([result,source,except](const ::mel::parallelism::BarrierData& ) mutable
                                {                
                                    if ( *except ) //any exception?
                                        result.setError(*except);
                                    else
                                        //result.assign(std::move(source.getValue())); //@todo it's not correct, but necesary to avoid a lot of copies. I left this way until solved in the root. Really is not very worrying
                                        result.assign(source);
                                    delete except;
                                    return ::mel::core::ECallbackResult::UNSUBSCRIBE; 
                                }));
                        }else
                        {
                            //set error as task in executor
                            launch(source.agent,[result,err = std::move(input.error())]( ) mutable noexcept
                            {
                                result.setError(std::move(err));
                            });
                        }
                    }
                    catch(...) 
                    {
                        result.setError( std::current_exception() );	
                    }
                }));
            return result;
        }

        ///@cond HIDDEN_SYMBOLS
        
        //void argument overload
        template <class ExecutorAgent,class I, class F>	 ExFuture<ExecutorAgent,void> loop(ExFuture<ExecutorAgent,void> source,I&& begin, I&& end, F&& functor, int increment = 1)
        {
            static_assert( std::is_invocable<F,int>::value, "execution::loop bad functor signature");
            ExFuture<ExecutorAgent,void> result(source.agent);
            typedef typename ExFuture<ExecutorAgent,void>::ValueType  ValueType;
            source.subscribeCallback(
                std::function<void(ValueType&)>([source,functor = std::forward<F>(functor),result,begin = std::forward<I>(begin),end = std::forward<I>(end),increment](ValueType& input)  mutable
                {
                    try
                    {   
                        if ( input.isValid() )
                        {                        
                            std::exception_ptr* except = new std::exception_ptr(nullptr);
                            ::mel::parallelism::Barrier barrier;
                            if constexpr (std::is_nothrow_invocable<F,I>::value)
                            {
                                barrier  = source.agent.loop(std::forward<I>(begin), std::forward<I>(end),
                                [f = std::forward<F>(functor),source](I idx) mutable noexcept
                                {
                                    f(idx);
                                }
                                , increment);
                            }else
                            {
                                barrier  = source.agent.loop(std::forward<I>(begin), std::forward<I>(end),
                                [f = std::forward<F>(functor),source,except](I idx) mutable
                                {
                                    try
                                    {
                                        f(idx);
                                    }catch(...)
                                    {
                                        if (!*except)
                                            *except = std::current_exception();
                                    }
                                }
                                , increment);
                            }

                            barrier.subscribeCallback(
                                std::function<::mel::core::ECallbackResult( const ::mel::parallelism::BarrierData&)>([result,source,except](const ::mel::parallelism::BarrierData& ) mutable
                                {                
                                    if ( *except ) //any exception?
                                        result.setError(*except);
                                    else
                                        //result.assign(std::move(source.getValue())); //@todo it's not correct, but necesary to avoid a lot of copies. I left this way until solved in the root. Really is not very worrying
                                        result.assign(source);
                                    delete except;
                                    return ::mel::core::ECallbackResult::UNSUBSCRIBE; 
                                }));                          
                        }else
                        {
                            //set error as task in executor
                            launch(source.agent,[result,err = std::move(input.error())]( )  mutable noexcept
                            {
                                result.setError(std::move(err));
                            });
                        }
                    }
                
                    catch(...) 
                    {
                        result.setError( std::current_exception() );	
                    }
                }));
            return result;
        }
        ///@endcond
        /**
         * @brief Execute given functions in a (possibly, depending on concrete executor) parallel way
         * If an exception is thrown in any of the callables, and noexcept is no specified, the
         * value of the first exception thrown is set as error in the resulting future, so forwarding the error to the next element (if any) of the chain
         @return A ExFuture with same value as input future, whose content IS MOVED
        */
        template <class ExecutorAgent,class TArg,class ...FTypes> ExFuture<ExecutorAgent,TArg> parallel(ExFuture<ExecutorAgent,TArg> source, FTypes&&... functions)
        {
            ExFuture<ExecutorAgent,TArg> result(source.agent);
            typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
            source.subscribeCallback(            
                //@note in C++20 I could have done fs... = std::forward<FTypes>(functions)..., but not in C++17
                std::function<void( ValueType&)>([source,result,fs = std::make_tuple(std::forward<FTypes>(functions)... )](ValueType& input)  mutable
                {
                    if ( input.isValid() )
                    {
                        std::exception_ptr* except = new std::exception_ptr(nullptr);
                        auto barrier  = source.agent.parallel(source,*except,std::forward<FTypes>(std::get<FTypes>(fs))...);
                        barrier.subscribeCallback(
                            std::function<::mel::core::ECallbackResult( const ::mel::parallelism::BarrierData&)>([source,result,except](const ::mel::parallelism::BarrierData& ) mutable
                            {                 
                                if ( *except ) //any exception?
                                    result.setError(*except);
                                else
                                    //result.assign(std::move(source.getValue()));//@todo it's not correct, but necesary to avoid a lot of copies. I left this way until solved in the root. Really is not very worrying
                                    result.assign(source);
                                delete except;
                                return ::mel::core::ECallbackResult::UNSUBSCRIBE; 
                            }));
                    }else
                    {
                        //set error as task in executor
                        launch(source.agent,[result,err = std::move(input.error())]( ) mutable noexcept
                        {
                            result.setError(std::move(err));
                        });
                    }
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
            ExFuture<ExecutorAgent,TArg> result(source.agent);
            source.subscribeCallback(
                //need to bind de source future to not get lost and input pointing to unknown place                
                std::function<void( ValueType&)>([source,f = std::forward<F>(f),result]( ValueType& input) mutable
                {       
                    if ( !input.isValid() )
                    {                
                        source.agent. template launch<TArg>([f=std::forward<F>(f)](ExFuture<ExecutorAgent,TArg>& arg) mutable noexcept(std::is_nothrow_invocable<F,std::exception_ptr>::value) -> TArg
                        {                                          
                            return f(arg.getValue().error());
                        },source,result);
                    }else
                        //result.assign(std::move(input));
                        result.assign(source);
                    
                })
            );
            return result;
        }           
        /**
         * @brief Same as @ref parallel but returning a tuple with the values for each functor
         * So, these functors must return a value
         * @return a tuple with types for each functor return, in order. Returning void is not allowed
         */
        template <class ResultTuple, class TArg,class ExecutorAgent,class ...FTypes> ExFuture<ExecutorAgent,ResultTuple> parallel_convert(ExFuture<ExecutorAgent,TArg> source, FTypes&&... functions)
        {
            //@todo tratar de dedudir la tupla de los resultados de cada funcion
            static_assert(std::is_default_constructible<ResultTuple>::value,"All types returned by the input ExFutures must be DefaultConstructible");
            ExFuture<ExecutorAgent,ResultTuple> result(source.agent);
            typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
            source.subscribeCallback(            
                std::function<void( ValueType&)>([source,result,fs = std::make_tuple(std::forward<FTypes>(functions)... )](ValueType& input)  mutable
                {
                    if ( input.isValid() )
                    {                       
                        std::exception_ptr* except = new std::exception_ptr(nullptr);
                        ResultTuple* output = new ResultTuple; //para que compile
                        auto barrier  = source.agent.parallel_convert(source,*except,*output,std::forward<FTypes>(std::get<FTypes>(fs))...);
                        barrier.subscribeCallback(
                            std::function<::mel::core::ECallbackResult( const ::mel::parallelism::BarrierData&)>([result,output,except](const ::mel::parallelism::BarrierData& ) mutable
                            {      
                                if ( *except ) //any exception?
                                    result.setError(*except);
                                else
                                    result.setValue(std::move(*output));
                                delete output;    
                                delete except;              
                                return ::mel::core::ECallbackResult::UNSUBSCRIBE; 
                            }));
                    }else
                    {
                        //set error as task in executor
                        launch(source.agent,[result,err = std::move(input.error())]( ) mutable noexcept
                        {
                        result.setError(std::move(err));
                        });
                    }
                }
            ));
            return result;
        }
        /**
         * @brief Get the Executor used in the chaing of execution
         * @details It returns an ExFuture moved from source, so transferring previous value to the next element in chain.
         * The intent of function is to be able to check executor traits or change some option in it
         * @param[in] f Callable with signature void (auto executor)
         * @see ExecutorTraits
         */
        template <class F,class TArg,class ExecutorAgent> ExFuture<ExecutorAgent,TArg> 
            getExecutor(ExFuture<ExecutorAgent,TArg> source, F&& f)
        {                
            static_assert(std::is_same<std::invoke_result_t<F,Executor<ExecutorAgent>>,void>::value,
                          "return value for callbable in 'getExecutor' must be void");
            typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
            ExFuture<ExecutorAgent,TArg> result(source.agent);            
            source.subscribeCallback(
                //need to bind de source future to not get lost and input pointing to unknown place                
                std::function<void( ValueType&)>([source,f = std::forward<F>(f),result]( ValueType& input) mutable
                {       
                    f(source.agent);
                    //result.assign(std::move(input));
                    result.assign(source);
                })
            );
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
                    return inmediate(fut,std::forward<TRet>(arg));
                }
            };
            template <class NewExecutionAgent> struct ApplyTransfer
            {
                ApplyTransfer(Executor<NewExecutionAgent>&& a):newAgent(std::forward<Executor<NewExecutionAgent>>(a)){}
                ApplyTransfer(const Executor<NewExecutionAgent>& a):newAgent(a){}
                Executor<NewExecutionAgent> newAgent;
                template <class TRet,class OldExecutionAgent> ExFuture<NewExecutionAgent,TRet> operator()(ExFuture<OldExecutionAgent,TRet> fut)
                {
                    return transfer(fut,newAgent);
                }
            };
            template <class F> struct ApplyNext
            {
                ApplyNext(F&& f):mFunc(std::forward<F>(f)){}
                ApplyNext(const F& f):mFunc(f){}
                F mFunc;
                template <class TArg,class ExecutorAgent> auto operator()(const ExFuture<ExecutorAgent,TArg>& inputFut)
                {
                    return next(inputFut,std::forward<F>(mFunc));
                }
                template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg>&& inputFut)
                {
                   // return next(std::move(inputFut),std::forward<F>(mFunc));
                   //@todo hacer las versiones &&
                   return next(inputFut,std::forward<F>(mFunc));
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
                    return loop(inputFut,begin,end,std::forward<F>(mFunc));
                }
            };
            
            template <class ...FTypes> struct ApplyBulk
            {
                ApplyBulk(FTypes&&... fs):mFuncs(std::forward<FTypes>(fs)...){}
                ApplyBulk(const FTypes&... fs):mFuncs(fs...){}           
                std::tuple<FTypes...> mFuncs;
                template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
                {
                    return parallel(inputFut,std::forward<FTypes>(std::get<FTypes>(mFuncs))...);
                }
            };
            template <class F> struct ApplyError
            {
                ApplyError(F&& f):mFunc(std::forward<F>(f)){}
                ApplyError(const F& f):mFunc(f){}
                F mFunc;
                template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
                {
                    return catchError(inputFut,std::forward<F>(mFunc));
                }
            };
            template <class F> struct ApplyGetExecutor
            {
                ApplyGetExecutor(F&& f):mFunc(std::forward<F>(f)){}
                ApplyGetExecutor(const F& f):mFunc(f){}
                F mFunc;
                template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
                {
                    return getExecutor(inputFut,std::forward<F>(mFunc));
                }
            };            
            template <int n,class TupleType,class FType> void _on_all(TupleType* tup,::mel::parallelism::Barrier& barrier, FType fut)
            {
                fut.subscribeCallback(
                    std::function<void( typename FType::ValueType&)>([tup,barrier](typename FType::ValueType& input)  mutable
                    {
                        std::get<n>(*tup) = std::move(input);
                        barrier.set();
                    }));
            }

            template <int n,class TupleType,class FType,class ...FTypes> void _on_all(TupleType* tup, ::mel::parallelism::Barrier& barrier,FType fut,FTypes... rest)
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
                    return parallel_convert<ReturnTuple,TArg>(inputFut,std::forward<FTypes>(std::get<FTypes>(mFuncs))...);
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
        ///@brief version for use with operator |
        template <class F> _private::ApplyGetExecutor<F> getExecutor(F&& f)
        {
            return _private::ApplyGetExecutor<F>(std::forward<F>(f));
        }
        ///@endcond

        /**
         * @brief overload operator | for chaining
        */
        template <class ExecutorAgent,class TRet1,class U> auto operator | (const ExFuture<ExecutorAgent,TRet1>& input,U&& u)
        {
            return u(input);
        } 
        template <class ExecutorAgent,class TRet1,class U> auto operator | (ExFuture<ExecutorAgent,TRet1>&& input,U&& u)
        {
            return u(std::move(input));
        } 

        /**
         * @brief Excepcion thrown by \ref ::mel::execution::on_all "on_all" when some of the futures raise error
         * @details It contains a reference to the causing exception and the element in the tuple which caused de exception
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
            ::mel::parallelism::Barrier barrier(sizeof...(futs));
            typedef std::tuple<typename FTypes::ValueType...>  _ttype;
            _ttype* tupleRes = new _ttype;

            barrier.subscribeCallback(
            std::function<::mel::core::ECallbackResult( const ::mel::parallelism::BarrierData&)>([result,tupleRes](const ::mel::parallelism::BarrierData& ) mutable
            {
                ReturnType resultVal;			
                auto r = ::mel::execution::_private::_moveValue<0>(*tupleRes,resultVal);
                if ( r == std::nullopt)
                {
                    result.setValue(std::move(resultVal));
                }else
                {
                    result.setError( std::make_exception_ptr(OnAllException(r.value().first,r.value().second,"Error in tuple element"))); 
                }
                delete tupleRes;
                return ::mel::core::ECallbackResult::UNSUBSCRIBE; 
            }));
            ::mel::execution::_private::_on_all<0,_ttype>(tupleRes,barrier,futs...);  //la idea es pasar una barrera o lo que sea y devolver resultado al activarse
            return result;
        }           
    }
}