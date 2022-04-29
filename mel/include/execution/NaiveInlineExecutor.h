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

        struct NaiveInlineExecutionAgent{};
        /**
         * @brief Executor specialization using NaiveInlineExecutionAgent as execution agent
         * @details Its behaviour is same as InlineExecutor, but made in the most easy way, while InlineExecutor has better performance 
         * (also it shouldn't be very importan) because it avoids unnecesary copies and moves by reimplementing base Executor functions
         */        
        template <> class Executor<NaiveInlineExecutionAgent>
        {
            public:                
                ///@{ 
                //! brief mandatory interface from Executor
                template <class TRet,class TArg,class F> void launch( F&& f,TArg&& arg,ExFuture<NaiveInlineExecutionAgent,TRet> output) const noexcept
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
                template <class TRet,class F> void launch( F&& f,ExFuture<NaiveInlineExecutionAgent,TRet> output) const noexcept
                {
                    if constexpr (std::is_same<std::invoke_result_t<F>,void>::value )
                    {
                        if constexpr (std::is_nothrow_invocable<F>::value)
                        {
                            f();
                            output.setValue();                            
                        }
                        else
                        {
                            try
                            {
                                f();
                                output.setValue();
                            }catch(...)
                            {
                                output.setError(std::current_exception());
                            }
                        }  
                    }else
                    {
                        if constexpr (std::is_nothrow_invocable<F>::value)
                            output.setValue(f());
                        else
                        {
                            try
                            {
                                output.setValue(f());
                            }catch(...)
                            {
                                output.setError(std::current_exception());
                            }
                        }                        
                    }                    
                }
                template <class I, class F>	 ::mel::parallelism::Barrier loop( I&& begin, I&& end, F&& functor, int increment);
                template <class TArg,class ...FTypes> ::mel::parallelism::Barrier parallel(ExFuture<NaiveInlineExecutionAgent,TArg> fut,std::exception_ptr& excpt, FTypes&&... functions);
                template <class ReturnTuple,class TArg,class ...FTypes> ::mel::parallelism::Barrier parallel_convert(ExFuture<NaiveInlineExecutionAgent,TArg> fut,std::exception_ptr& excpt,ReturnTuple& result, FTypes&&... functions);
                ///@}
        };  
        namespace _naive{namespace _private
        {
            template <class F,class TArg> void _invokeInline(ExFuture<NaiveInlineExecutionAgent,TArg> fut,std::exception_ptr& except,F&& f)
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
            template <class F> void _invokeInline(ExFuture<NaiveInlineExecutionAgent,void> fut,std::exception_ptr& except,F&& f)
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
            template <class TArg,class F,class ...FTypes> void _invokeInline(ExFuture<NaiveInlineExecutionAgent,TArg> fut,std::exception_ptr& except,F&& f, FTypes&&... fs)
            {            
                _naive::_private::_invokeInline(fut,except,std::forward<F>(f));
                _naive::_private::_invokeInline(fut,except,std::forward<FTypes>(fs)...);
            }
            template <int n,class ResultTuple, class F,class TArg> void _invokeInline_with_result(ExFuture<NaiveInlineExecutionAgent,TArg> fut,std::exception_ptr& except,ResultTuple& output,F&& f)
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
            template <int n,class ResultTuple,class F> void _invokeInline_with_result(ExFuture<NaiveInlineExecutionAgent,void>& fut,std::exception_ptr& except,ResultTuple& output, F&& f)
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
            
            template <int n,class ResultTuple,class TArg,class F,class ...FTypes> void _invokeInline_with_result(ExFuture<NaiveInlineExecutionAgent,TArg> fut,std::exception_ptr& except,ResultTuple& output,F&& f, FTypes&&... fs)
            {            
                _naive::_private::_invokeInline_with_result<n>(fut,except,output,std::forward<F>(f));
                _naive::_private::_invokeInline_with_result<n+1>(fut,except,output,std::forward<FTypes>(fs)...);
            }            
        }}      
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
        template <class I, class F>	 ::mel::parallelism::Barrier Executor<NaiveInlineExecutionAgent>::loop(I&& begin, I&& end, F&& functor, int increment)
        {
            //@todo no correcto con no random iterators
            for(auto i = begin; i != end;i+=increment)
            {
                functor(i);
            }               
            return ::mel::parallelism::Barrier((size_t)0);
        }
        template <class TArg,class ...FTypes> ::mel::parallelism::Barrier Executor<NaiveInlineExecutionAgent>::parallel( ExFuture<NaiveInlineExecutionAgent,TArg> fut,std::exception_ptr& except,FTypes&&... functions)
        {            
            _naive::_private::_invokeInline(fut,except,functions...);
            return ::mel::parallelism::Barrier((size_t)0);
        }    
        template <class ReturnTuple,class TArg,class ...FTypes> ::mel::parallelism::Barrier Executor<NaiveInlineExecutionAgent>::parallel_convert(ExFuture<NaiveInlineExecutionAgent,TArg> fut,std::exception_ptr& except,ReturnTuple& result, FTypes&&... functions)
        {
            _naive::_private::_invokeInline_with_result<0>(fut,except,result,functions...);
            return ::mel::parallelism::Barrier((size_t)0);
        }
        
        /**
         * @brief Executor Traits for NaiveInlineExecutionAgent Executor
         */
        template <> struct ExecutorTraits<Executor<NaiveInlineExecutionAgent>>
        {
            enum {has_microthreading = false};  //support microthreading?
            enum {has_parallelism = false}; ////support true parallelism?
        };
        //! @brief alias for Executor<NaiveInlineExecutionAgent>
        typedef Executor<NaiveInlineExecutionAgent> NaiveInlineExecutor; //alias
    }
}