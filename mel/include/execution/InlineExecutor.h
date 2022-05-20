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
        using namespace mel::execution;
        struct InlineExecutionAgent{};
        /**
         * @brief Executor specialization using InlineExecutionAgent as execution agent
         * @details This executor behaves as if execution is done straightforward in-place.
         */        
        template <> class Executor<InlineExecutionAgent>
        {                           
        };  
        namespace _private
        {
            template <class F,class TArg> void _invokeInline(ExFuture<InlineExecutionAgent,TArg> fut,std::exception_ptr& except,F&& f)
            {
                static_assert( std::is_invocable<F,TArg>::value, "inlineExecutor::_invokeInline bad signature");
                if constexpr (std::is_nothrow_invocable<F,TArg>::value)
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
                static_assert( std::is_invocable<F,TArg>::value, "inlineExecutor::_invokeInline_with_result bad signature");
                if constexpr (std::is_nothrow_invocable<F,TArg>::value)
                {
                    if constexpr (std::is_same< std::invoke_result_t<F,TArg>,void >::value)
                        f(fut.getValue().value());
                    else
                        std::get<n>(output) = f(fut.getValue().value());
                }else
                {
                    try
                    {
                        if constexpr (std::is_same< std::invoke_result_t<F,TArg>,void >::value)
                            f(fut.getValue().value());
                        else
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
                    if constexpr (std::is_same< std::invoke_result_t<F>,void >::value)
                        f();
                    else
                        std::get<n>(output) = f();
                }else
                {
                    try
                    {
                        if constexpr (std::is_same< std::invoke_result_t<F>,void >::value)
                            f();
                        else
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
        
        // overload for performance reasons
        template <class TArg, class I, class F>	 ExFuture<InlineExecutionAgent,TArg> loop(ExFuture<InlineExecutionAgent,TArg> source,I&& begin, I&& end, F&& functor, int increment = 1)
        {
            static_assert( std::is_invocable<F,I,TArg>::value, "InlineExecutor::loop bad signature");
            if ( source.getValid())
            {
                std::exception_ptr except{nullptr};
                auto& v = source.getValue().value();
                if constexpr (std::is_nothrow_invocable<F,I,TArg>::value)
                {
                    for(auto i = begin; i != end;i+=increment)
                    {
                        functor(i,v);
                    }   
                }else
                {
                    for(auto i = begin; i != end;i+=increment)
                    {                    
                        try
                        {
                            functor(i,v);
                        }catch(...)
                        {
                            if ( !except )
                                except = std::current_exception();
                        }
                    }  
                }
                if ( !except )                
                    return ExFuture<InlineExecutionAgent,TArg>(source.agent,v);
                else
                {
                    auto result = ExFuture<InlineExecutionAgent,TArg>(source.agent);
                    result.setError(except);
                    return result;
                }
            }else
            {
                auto result = ExFuture<InlineExecutionAgent,TArg>(source.agent);
                result.setError(source.getValue().error());
                return result;
            }
        }
        // voide overload for performance reasons
        template <class I, class F>	 ExFuture<InlineExecutionAgent,void> loop(ExFuture<InlineExecutionAgent,void> source,I&& begin, I&& end, F&& functor, int increment = 1)
        {
            if ( source.getValid())
            {
                std::exception_ptr except{nullptr};
                if constexpr (std::is_nothrow_invocable<F,I>::value)
                {
                    for(auto i = begin; i != end;i+=increment)
                    {
                        functor(i);
                    }   
                }else
                {
                    for(auto i = begin; i != end;i+=increment)
                    {                    
                        try
                        {
                            functor(i);
                        }catch(...)
                        {
                            if ( !except )
                                except = std::current_exception();
                        }
                    }  
                }
                if ( !except )                
                    return ExFuture<InlineExecutionAgent,void>(source.agent,1);
                else
                {
                    auto result = ExFuture<InlineExecutionAgent,void>(source.agent);
                    result.setError(except);
                    return result;
                }
            }else
            {
                auto result = ExFuture<InlineExecutionAgent,void>(source.agent);
                result.setError(source.getValue().error());
                return result;
            }
        }
        template <class TArg,class ...FTypes> ExFuture<InlineExecutionAgent,TArg> parallel(ExFuture<InlineExecutionAgent,TArg> source, FTypes&&... functions)
        {            
            std::exception_ptr except{nullptr};
            if ( source.getValid() )
            {
                _private::_invokeInline(source,except,std::forward<FTypes>(functions)...);
                if ( !except)
                    return ExFuture<InlineExecutionAgent,TArg>(source.agent,source.getValue().value());
                else
                {
                    auto result = ExFuture<InlineExecutionAgent,TArg>(source.agent);
                    result.setError(except);
                    return result;
                }
            }
            else
            {
                auto result = ExFuture<InlineExecutionAgent,TArg>(source.agent);
                result.setError(source.getValue().error());
                return result;
            }
        } 
        /*   
        ///void overload
        template <class ...FTypes> ExFuture<InlineExecutionAgent,void> parallel(ExFuture<InlineExecutionAgent,void> source, FTypes&&... functions)  
        {            
            std::exception_ptr except{nullptr};
            if ( source.getValid() )
            {
                _private::_invokeInline(source,except,std::forward<FTypes>(functions)...);
                if ( !except)
                    return ExFuture<InlineExecutionAgent,void>(source.agent,1);
                else
                {
                    auto result = ExFuture<InlineExecutionAgent,void>(source.agent);
                    result.setError(except);
                    return result;
                }
            }
            else
            {
                auto result = ExFuture<InlineExecutionAgent,void>(source.agent);
                result.setError(source.getValue().error());
                return result;
            }
        }   
        */
        //overload for performance reasons
        template <class TArg,class ...FTypes> ExFuture<InlineExecutionAgent,typename mel::execution::_private::GetReturn<TArg,FTypes...>::type> parallel_convert(ExFuture<InlineExecutionAgent,TArg> source, FTypes&&... functions)
        {            
            typedef typename mel::execution::_private::GetReturn<TArg,FTypes...>::type ResultTuple;
            std::exception_ptr except{nullptr};
            if ( source.getValid() )
            {             
                ExFuture<InlineExecutionAgent,ResultTuple> result(source.agent);
                ResultTuple resultTuple;
                _private::_invokeInline_with_result<0>(source,except,resultTuple,std::forward<FTypes>(functions)...);
                if ( !except)
                    result.setValue(std::move(resultTuple));
                else
                {
                    result.setError(except);
                }
                return result;
            }
            else
            {
                auto result = ExFuture<InlineExecutionAgent,ResultTuple>(source.agent);
                result.setError(source.getValue().error());
                return result;
            }
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
            static_assert( std::is_invocable<F,TArg>::value, "InlineExecutor::launch. Bad signature");               
            typedef std::invoke_result_t<F,TArg> TRet;
            //return ExFuture<InlineExecutionAgent,TRet>(ex,f(std::forward<TArg>(arg)));
            if constexpr (std::is_same<TRet,void>::value )
            {
                if constexpr (std::is_nothrow_invocable<F,TArg>::value)
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
                if constexpr (std::is_nothrow_invocable<F,TArg>::value)
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

        //reimplementation of base next for InlineExecutionAgent to improve performance compared to NaiveInlineExecutor
        template <class F,class TArg> ExFuture<InlineExecutionAgent,std::invoke_result_t<F,TArg>> 
            next(ExFuture<InlineExecutionAgent,TArg> source, F&& f)
        {   
            static_assert( std::is_invocable<F,TArg>::value, "InlineExecutor::next. Bad signature");
            typedef std::invoke_result_t<F,TArg> TRet;
            if ( source.getValid() )
            {
                if constexpr (std::is_same<TRet,void>::value )
                {
                    if constexpr (std::is_nothrow_invocable<F,TArg>::value)
                    {                            
                        f(source.getValue().value());
                        return ExFuture<InlineExecutionAgent,TRet>(source.agent,1);                   
                    }
                    else
                    {
                        try
                        {
                            f(source.getValue().value());
                            return ExFuture<InlineExecutionAgent,TRet>(source.agent,1);
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
                    if constexpr (std::is_nothrow_invocable<F,TArg>::value)
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
                }
                
            }else
            {
               // return ExFuture<InlineExecutionAgent,TRet>(source.agent,source.getValue().error());
               auto result = ExFuture<InlineExecutionAgent,TRet>(source.agent);
               result.setError(source.getValue().error());
               return result;
            }          
        }
        // void overload
        template <class F> ExFuture<InlineExecutionAgent,std::invoke_result_t<F>> 
            next(ExFuture<InlineExecutionAgent,void> source, F&& f)
        {                            
            typedef std::invoke_result_t<F> TRet;
            if ( source.getValid() )
            {
                if constexpr (std::is_same<TRet,void>::value )
                {
                    if constexpr (std::is_nothrow_invocable<F>::value)
                    {                            
                        f();
                        return ExFuture<InlineExecutionAgent,TRet>(source.agent,1);                   
                    }
                    else
                    {
                        try
                        {
                            f();
                            return ExFuture<InlineExecutionAgent,TRet>(source.agent,1);
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
                    if constexpr (std::is_nothrow_invocable<F>::value)
                    {                            
                        return ExFuture<InlineExecutionAgent,TRet>(source.agent,f());                   
                    }
                    else
                    {
                        try
                        {
                            return ExFuture<InlineExecutionAgent,TRet>(source.agent,f());
                        }catch(...)
                        {
                            //output.setError(std::current_exception());
                            auto result = ExFuture<InlineExecutionAgent,TRet>(source.agent);
                            result.setError(std::current_exception());
                            return result;
                        }
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

        //inmediate overload for performance reasons
        template <class TArg,class TRet> 
            ExFuture<InlineExecutionAgent,typename std::remove_cv<typename std::remove_reference<TRet>::type>::type> inmediate( ExFuture<InlineExecutionAgent,TArg> fut,TRet&& arg)
        {
            static_assert( !std::is_lvalue_reference<TRet>::value ||
                            std::is_const< typename std::remove_reference<TRet>::type>::value,"execution::inmediate. Use std::ref() to pass argument as reference");
            using NewType = typename std::remove_cv<typename std::remove_reference<TRet>::type>::type;
            return ExFuture<InlineExecutionAgent,NewType>(fut.agent,std::forward<TRet>(arg));           
        }

        /**
         * @brief Executor Traits for InlineExecutionAgent Executor
         */
        template <> struct ExecutorTraits<Executor<InlineExecutionAgent>> : ExecutorTraits<void>   //inherit same default traits
        {        
        };
        //! @brief alias for Executor<InlineExecutionAgent>
        typedef Executor<InlineExecutionAgent> InlineExecutor; //alias
    }
}