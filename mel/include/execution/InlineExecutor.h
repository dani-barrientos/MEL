#pragma once
/*
 * SPDX-FileCopyrightText: 2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
#include <execution/Executor.h>

#include <mpl/TypeTraits.h>
namespace mel
{
    namespace execution
    {                   
        struct InlineExecutionAgent{};
        /**
         * @brief Executor specialization using InlineExecutionAgent as execution agent
         * @details This executor behaves as if execution is done straightforward in-place.
         */        
        template <> class Executor<InlineExecutionAgent>
        {
            public:                
                ///@{ 
                //! brief mandatory interface from Executor
                template <class TRet,class TArg,class F> void launch( F&& f,TArg&& arg,ExFuture<InlineExecutionAgent,TRet> output) const noexcept
                {
                    if constexpr (std::is_same<std::invoke_result_t<F,TArg&>,void>::value )
                    {
                        if constexpr (std::is_nothrow_invocable<F,TArg&>::value)
                        {                            
                            f(std::forward<TArg>(arg));
                            output.setValue();                            
                        }
                        else
                        {
                            try
                            {
                                f(std::forward<TArg>(arg));
                                output.setValue();
                            }catch(...)
                            {
                                output.setError(std::current_exception());
                            }
                        }  
                    }else
                    {
                        if constexpr (std::is_nothrow_invocable<F,TArg&>::value)
                            output.setValue(f(std::forward<TArg>(arg)));
                        else
                        {
                            try
                            {
                                output.setValue(f(std::forward<TArg>(arg)));
                            }catch(...)
                            {
                                output.setError(std::current_exception());
                            }
                        }                        
                    }
                }
                
                template <class I, class F>	 ::mel::parallelism::Barrier loop( I&& begin, I&& end, F&& functor, int increment);
                template <class TArg,class ...FTypes> ::mel::parallelism::Barrier parallel(ExFuture<InlineExecutionAgent,TArg> fut,std::exception_ptr& excpt, FTypes&&... functions);
                template <class ReturnTuple,class TArg,class ...FTypes> ::mel::parallelism::Barrier parallel_convert(ExFuture<InlineExecutionAgent,TArg> fut,std::exception_ptr& excpt,ReturnTuple& result, FTypes&&... functions);
                ///@}
        };  
        namespace _private
        {
            template <class F,class TArg> void _invokeInline(ExFuture<InlineExecutionAgent,TArg> fut,std::exception_ptr& except,F&& f)
            {
                if constexpr (std::is_nothrow_invocable<F,TArg&>::value)
                {
                    f(fut.getValue().value());                    
                }else
                {
                    try
                    {
                        f(fut.getValue().value());
                    }catch(...)
                    {
                        if ( !except )
                            except = std::current_exception();
                    }
                }
            }
            //void overload
            template <class F> void _invokeInline(ExFuture<InlineExecutionAgent,void> fut,std::exception_ptr& except,F&& f)
            {
                if constexpr (std::is_nothrow_invocable<F>::value)
                {
                    f();                    
                }else
                {
                    try
                    {
                        f();
                    }catch(...)
                    {
                        if (!except)
                            except = std::current_exception();
                    }                                            
                }
            }
            template <class TArg,class F,class ...FTypes> void _invokeInline(ExFuture<InlineExecutionAgent,TArg> fut,std::exception_ptr& except,F&& f, FTypes&&... fs)
            {            
                _invokeInline(fut,except,std::forward<F>(f));
                _invokeInline(fut,except,std::forward<FTypes>(fs)...);
            }
            template <int n,class ResultTuple, class F,class TArg> void _invokeInline_with_result(ExFuture<InlineExecutionAgent,TArg> fut,std::exception_ptr& except,ResultTuple& output,F&& f)
            {
                if constexpr (std::is_nothrow_invocable<F,TArg&>::value)
                {
                    std::get<n>(output) = f(fut.getValue().value());                    
                }else
                {
                    try
                    {
                        std::get<n>(output) = f(fut.getValue().value());
                    }catch(...)
                    {
                        if ( !except )
                            except = std::current_exception();
                    }
                }
            
            }
            //void overload
            template <int n,class ResultTuple,class F> void _invokeInline_with_result(ExFuture<InlineExecutionAgent,void>& fut,std::exception_ptr& except,ResultTuple& output, F&& f)
            {
                if constexpr (std::is_nothrow_invocable<F>::value)
                {
                    std::get<n>(output) = f();                    
                }else
                {
                    try
                    {
                        std::get<n>(output) = f();
                    }catch(...)
                    {
                        if (!except)
                            except = std::current_exception();
                    }                                            
                }             
            }
            
            template <int n,class ResultTuple,class TArg,class F,class ...FTypes> void _invokeInline_with_result(ExFuture<InlineExecutionAgent,TArg> fut,std::exception_ptr& except,ResultTuple& output,F&& f, FTypes&&... fs)
            {            
                _invokeInline_with_result<n>(fut,except,output,std::forward<F>(f));
                _invokeInline_with_result<n+1>(fut,except,output,std::forward<FTypes>(fs)...);
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
        template <class I, class F>	 ::mel::parallelism::Barrier Executor<InlineExecutionAgent>::loop(I&& begin, I&& end, F&& functor, int increment)
        {
            //@todo no correcto con no random iterators
            for(auto i = begin; i != end;i+=increment)
            {
                functor(i);
            }               
            return ::mel::parallelism::Barrier((size_t)0);
        }
        template <class TArg,class ...FTypes> ::mel::parallelism::Barrier Executor<InlineExecutionAgent>::parallel( ExFuture<InlineExecutionAgent,TArg> fut,std::exception_ptr& except,FTypes&&... functions)
        {            
            _private::_invokeInline(fut,except,functions...);
            return ::mel::parallelism::Barrier((size_t)0);
        }    
        template <class ReturnTuple,class TArg,class ...FTypes> ::mel::parallelism::Barrier Executor<InlineExecutionAgent>::parallel_convert(ExFuture<InlineExecutionAgent,TArg> fut,std::exception_ptr& except,ReturnTuple& result, FTypes&&... functions)
        {
            _private::_invokeInline_with_result<0>(fut,except,result,functions...);
            return ::mel::parallelism::Barrier((size_t)0);
        }
        /**
         * @brief Launch given functor in given executor
         * @return ExFuture with return type of function
         */
        template <class F> ExFuture<InlineExecutionAgent,std::invoke_result_t<F>> launch( Executor<InlineExecutionAgent> ex,F&& f)
        {   
            typedef std::invoke_result_t<F> TRet;
            
            if constexpr (std::is_same<TRet,void>::value )
            {
                if constexpr (std::is_nothrow_invocable<F>::value)
                {
                    f();
                    return ExFuture<InlineExecutionAgent,TRet>(ex,1);
                }
                else
                {
                    try
                    {
                        f();
                        return ExFuture<InlineExecutionAgent,TRet>(ex,1);
                    }catch(...)
                    {
                        auto result = ExFuture<InlineExecutionAgent,TRet>(ex);
                        result.setError(std::current_exception());
                        return result;
                    }
                }  
            }else
            {
                if constexpr (std::is_nothrow_invocable<F>::value)
                    return ExFuture<InlineExecutionAgent,TRet>(ex,f());
                else
                {
                    try
                    {
                        return ExFuture<InlineExecutionAgent,TRet>(ex,f());
                    }catch(...)
                    {
                        auto result = ExFuture<InlineExecutionAgent,TRet>(ex);
                        result.setError(std::current_exception());
                        return result;
                    }
                }                        
            }                 
        }
        /**
         * @brief Launch given functor in given executor, passing it input parameter
         * @return ExFuture with return type of function
         */
        template <class TArg,class F> ExFuture<InlineExecutionAgent,std::invoke_result_t<F,TArg>> launch( Executor<InlineExecutionAgent> ex,F&& f,TArg&& arg)
        {
            /*
            @todo I need to mature this idea. It's not so transparent to add reference check
            but same rules as for "inmediate" should be followed
            static_assert( !std::is_lvalue_reference<TArg>::value ||
                std::is_const< typename std::remove_reference<TArg>::type>::value,"execution::launch. Use std::ref() to pass argument as reference");
                */
            typedef std::invoke_result_t<F,TArg> TRet;
            //return ExFuture<InlineExecutionAgent,TRet>(ex,f(std::forward<TArg>(arg)));
            if constexpr (std::is_same<TRet,void>::value )
            {
                if constexpr (std::is_nothrow_invocable<F>::value)
                {
                    f(std::forward<TArg>(arg));
                    return ExFuture<InlineExecutionAgent,TRet>(ex,1);
                }
                else
                {
                    try
                    {
                        f(std::forward<TArg>(arg));
                        return ExFuture<InlineExecutionAgent,TRet>(ex,1);
                    }catch(...)
                    {
                        auto result = ExFuture<InlineExecutionAgent,TRet>(ex);
                        result.setError(std::current_exception());
                        return result;
                    }
                }  
            }else
            {
                if constexpr (std::is_nothrow_invocable<F>::value)
                    return ExFuture<InlineExecutionAgent,TRet>(ex,f(std::forward<TArg>(arg)));
                else
                {
                    try
                    {
                        return ExFuture<InlineExecutionAgent,TRet>(ex,f(std::forward<TArg>(arg)));
                    }catch(...)
                    {
                        auto result = ExFuture<InlineExecutionAgent,TRet>(ex);
                        result.setError(std::current_exception());
                        return result;
                    }
                }                        
            }     
            
        }

        //reimplementation of base next for InlineExecutionAgent to improve better performance compared to NaiveInlineExecutor
        template <class F,class TArg> ExFuture<InlineExecutionAgent,std::invoke_result_t<F,TArg&>> 
            next(ExFuture<InlineExecutionAgent,TArg> source, F&& f)
        {   
            
            typedef std::invoke_result_t<F,TArg&> TRet;
            if ( source.getValid() )
            {
                if constexpr (std::is_nothrow_invocable<F,TArg&>::value)
                {                            
                    return ExFuture<InlineExecutionAgent,TRet>(source.agent,f(source.getValue().value()));                   
                }
                else
                {
                    try
                    {
                        return ExFuture<InlineExecutionAgent,TRet>(source.agent,f(source.getValue().value()));
                    }catch(...)
                    {
                        //output.setError(std::current_exception());
                        auto result = ExFuture<InlineExecutionAgent,TRet>(source.agent);
                        result.setError(std::current_exception());
                        return result;
                    }
                }  
            }else
            {
               // return ExFuture<InlineExecutionAgent,TRet>(source.agent,source.getValue().error());
               auto result = ExFuture<InlineExecutionAgent,TRet>(source.agent);
               result.setError(source.getValue().error());
               return result;
            }          
        }
        /**
         * @brief Executor Traits for InlineExecutionAgent Executor
         */
        template <> struct ExecutorTraits<Executor<InlineExecutionAgent>>
        {
            enum {has_microthreading = false};  //support microthreading?
            enum {has_parallelism = false}; ////support true parallelism?
        };
        //! @brief alias for Executor<InlineExecutionAgent>
        typedef Executor<InlineExecutionAgent> InlineExecutor; //alias
    }
}