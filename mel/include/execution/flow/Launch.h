#pragma once
/*
 * SPDX-FileCopyrightText: 2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
#include <execution/Executor.h>
#include <preprocessor/utils.h>
#include <tuple>
namespace mel
{
    namespace execution
    {
        namespace flow
        {
            ///!brief tuple wrapper for holding result of flows execution
            template <class TupleType> struct FlowsResult
            {
                using type = TupleType;
                FlowsResult( TupleType&& tt ) : tup( std::move( tt ) ){};
                TupleType tup;
            };
            ///@cond HIDDEN_SYMBOLS
            namespace _private
            {
                // helper for invoking flows
                template <int n, class ResultTuple, class Flow,
                          class ExecutionAgent, class TArg>
                void _invokeFlow( ExFuture<ExecutionAgent, TArg> fut,
                                  std::exception_ptr& except,
                                  ResultTuple& output, Flow&& f )
                {
                    // static_assert( std::is_invocable<F,TArg>::value,
                    // "inlineExecutor::_invokeInline bad signature");
                    if constexpr ( std::is_nothrow_invocable<
                                       Flow,
                                       ExFuture<ExecutionAgent, TArg>>::value )
                    {
                        std::get<n>( output ) = f( fut );
                    }
                    else
                    {
                        try
                        {
                            std::get<n>( output ) = f( fut );
                        }
                        catch ( ... )
                        {
                            if ( !except )
                                except = std::current_exception();
                        }
                    }
                }
                template <int n, class ResultTuple, class ExecutionAgent,
                          class TArg, class Flow, class... Flows>
                void _invokeFlow( ExFuture<ExecutionAgent, TArg> fut,
                                  std::exception_ptr& except,
                                  ResultTuple& output, Flow&& f, Flows&&... fs )
                {
                    _invokeFlow<n>( fut, except, output,
                                    std::forward<Flow>( f ) );
                    _invokeFlow<n + 1>( fut, except, output,
                                        std::forward<Flows>( fs )... );
                }
                template <class ExecutionAgent, class T, size_t... Is>
                auto _forwardOnAll( Executor<ExecutionAgent> ex, T&& tup,
                                    std::index_sequence<Is...> )
                {
                    return execution::on_all( ex, std::get<Is>( tup )... );
                }
            }; // namespace _private
            ///@endcond

            /**
             * @brief Launch given set of flows
             *
             * @param source previous result in current job
             * @param flows callables with the form ExFuture f(ExFuture)
             * @return a std::tuple with the ExFuture result of each flow (in
             * order)
             */
            template <class TArg, class ExecutorAgent, class... Flows>
            FlowsResult<typename ::mel::execution::_private::GetReturn<
                ExFuture<ExecutorAgent, TArg>, Flows...>::type>
            launch( ExFuture<ExecutorAgent, TArg> source, Flows... flows )
            {
                typedef typename ::mel::execution::_private::GetReturn<
                    ExFuture<ExecutorAgent, TArg>, Flows...>::type ResultTuple;
                ResultTuple output;
                std::exception_ptr
                    except; //@todo, uhmm, no es muy importante, porque se
                            // refiere a error en la funcion que lanza el flow..
                _private::_invokeFlow<0>( source, except, output,
                                          std::move( flows )... );
                return FlowsResult<ResultTuple>( std::move( output ) );
            }
            /**
             * @brief Takes a tuple with the results of execution of some flows
             * and does a execution::on_all
             */
            template <class ExecutionAgent, class TupleFlow>
            auto on_all( Executor<ExecutionAgent> ex, TupleFlow&& f )
            {
                // constexpr size_t ts = std::tuple_size<typename
                // std::remove_reference<TupleFlow>::type>::value; return
                // _private::_forwardOnAll(ex,f,std::make_index_sequence<ts>{});
                constexpr size_t ts = std::tuple_size<
                    typename std::remove_reference_t<TupleFlow>::type>::value;
                return _private::_forwardOnAll(
                    ex, f.tup, std::make_index_sequence<ts>{} );
            }

            namespace _private
            {
                template <class ExecutionAgent> struct ApplyOnAll
                {
                    ApplyOnAll( Executor<ExecutionAgent> ex ) : mEx( ex ) {}
                    Executor<ExecutionAgent> mEx;

                    template <class TupleType>
                    auto operator()( FlowsResult<TupleType>&& fr )
                    {
                        return on_all( mEx, std::move( fr ) );
                    }
                    template <class TupleType>
                    auto operator()( const FlowsResult<TupleType>& fr )
                    {
                        return on_all( mEx, fr );
                    }
                };

                template <class... Flows> struct ApplyLaunch
                {
                    template <class... Fs>
                    ApplyLaunch( Fs&&... fs )
                        : mFuncs( std::forward<Fs>( fs )... )
                    {
                    }
                    std::tuple<Flows...> mFuncs;
                    template <class TArg, class ExecutorAgent>
                    auto operator()( ExFuture<ExecutorAgent, TArg> inputFut )
                    {
                        return launch( inputFut,
                                       std::forward<Flows>(
                                           std::get<Flows>( mFuncs ) )... );
                    }
                };
            } // namespace _private

            ///@brief version for use with operator |
            template <class... Flows>
            _private::ApplyLaunch<Flows...> launch( Flows&&... flows )
            {
                return _private::ApplyLaunch<Flows...>(
                    std::forward<Flows>( flows )... );
            }
            template <class ExecutionAgent>
            auto on_all( Executor<ExecutionAgent> ex )
            {
                return _private::ApplyOnAll<ExecutionAgent>( ex );
            }
            /**
             * @brief overload operator | for chaining a FlowsResult
             */
            template <class TupleType, class U>
            auto operator|( const FlowsResult<TupleType>& input, U&& u )
            {
                return u( input );
            }
            template <class TupleType, class U>
            auto operator|( FlowsResult<TupleType>&& input, U&& u )
            {
                return u( std::move( input ) );
            }

        } // namespace flow
    }     // namespace execution
} // namespace mel