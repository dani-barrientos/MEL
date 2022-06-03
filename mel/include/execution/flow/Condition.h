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
        /**
         * @brief Functions for flow management
         */
        namespace flow
        {
            // for internal use by condition function
#define CONDITION_SELECT_JOB( idx )                                            \
    if constexpr ( tsize > idx )                                               \
    {                                                                          \
        using FlowType = std::tuple_element_t<idx, TupleType>;                 \
        static_assert(                                                         \
            std::is_invocable<FlowType, ExFuture<ExecutorAgent, TArg>>::value, \
            "execution::condition bad functor signature" );                    \
        launch( source.agent,                                                  \
                [source, result, fls = std::move( fls )]() mutable noexcept    \
                {                                                              \
                    if constexpr ( std::is_nothrow_invocable<                  \
                                       FlowType,                               \
                                       ExFuture<ExecutorAgent, TArg>>::value ) \
                        result.assign( std::get<idx>( fls )( source ) );       \
                    else                                                       \
                    {                                                          \
                        try                                                    \
                        {                                                      \
                            result.assign( std::get<idx>( fls )( source ) );   \
                        }                                                      \
                        catch ( ... )                                          \
                        {                                                      \
                            result.setError( std::current_exception() );       \
                        }                                                      \
                    }                                                          \
                } );                                                           \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        launch( source.agent,                                                  \
                [result]() mutable noexcept                                    \
                {                                                              \
                    result.setError( std::out_of_range(                        \
                        "execution::condition. Index '" TOSTRING(              \
                            idx ) "' is greater than maximum case "            \
                                  "index " TOSTRING( tsize ) ) );              \
                } );                                                           \
    }
            /**
             * @brief Select functor to execute
             * @details Callable \p selector return an unsigned int with is the
             * index of de callable in \p jobs If index is greater than
             * available callables, an std::out_of_range error is set
             * @param selector callable with signature 'size_t f(TArg)'
             * @param flows variable number of callables to choose in \p
             * selector
             */
            template <class ExecutorAgent, class TArg, class F, class... Flows>
            auto condition( ExFuture<ExecutorAgent, TArg> source, F selector,
                            Flows... flows )
            {

                typedef
                    typename ExFuture<ExecutorAgent, TArg>::ValueType ValueType;
                typedef typename ::mel::execution::_private::GetReturn<
                    ExFuture<ExecutorAgent, TArg>, Flows...>::type ResultTuple;
                using ResultType = std::tuple_element_t<0, ResultTuple>;
                ResultType result( source.agent );
                source.subscribeCallback(
                    // need to bind de source future to not get lost and input
                    // pointing to unknown place

                    [source, selector = std::move( selector ),
                     fls = std::make_tuple( std::move( flows )... ), result](
                        ValueType&
                            input ) mutable noexcept( std::
                                                          is_nothrow_invocable<
                                                              F, TArg>::value )
                    {
                        // lanzar tarea para deteccion noexcept, ¿para todo
                        // junto? creo que sí...
                        if ( input.isValid() )
                        {
                            using TupleType = decltype( fls );
                            // Evaluate index
                            size_t idx = selector( input.value() );
                            constexpr size_t tsize =
                                std::tuple_size<TupleType>::value;
                            switch ( idx )
                            {
                            case 0:

                                // //codigo a pelo para temas de depuracion
                                // if constexpr (tsize>0)
                                // {
                                //     using FlowType =
                                //     std::tuple_element_t<0,TupleType>;
                                //     launch(source.agent,[source,result,fls =
                                //     std::move(fls)]() mutable noexcept
                                //         {
                                //             if constexpr
                                //             (std::is_nothrow_invocable<FlowType,ExFuture<ExecutorAgent,TArg>>::value)
                                //                 result.assign(std::get<0>(fls)(source));
                                //             else
                                //             {
                                //                 try
                                //                 {
                                //                     result.assign(std::get<0>(fls)(source));
                                //                 }catch(...)
                                //                 {
                                //                     result.setError(std::current_exception());
                                //                 }
                                //             }
                                //         }
                                //      );
                                // }else{
                                //     launch(source.agent,[result]( ) mutable
                                //     noexcept {
                                //         result.setError(std::out_of_range("triqui"));
                                //     });
                                // }
                                CONDITION_SELECT_JOB( 0 )
                                break;
                            case 1:
                                CONDITION_SELECT_JOB( 1 )
                                break;
                            case 2:
                                CONDITION_SELECT_JOB( 2 )
                                break;
                            case 3:
                                CONDITION_SELECT_JOB( 3 )
                                break;
                            case 4:
                                CONDITION_SELECT_JOB( 4 )
                                break;
                            case 5:
                                CONDITION_SELECT_JOB( 5 )
                                break;
                            case 6:
                                CONDITION_SELECT_JOB( 6 )
                                break;
                            case 7:
                                CONDITION_SELECT_JOB( 7 )
                                break;
                            case 8:
                                CONDITION_SELECT_JOB( 8 )
                                break;
                            case 9:
                                CONDITION_SELECT_JOB( 9 )
                                break;
                            }
                        }
                        else
                        {
                            // set error as task in executor
                            std::exception_ptr err = input.error();
                            launch( source.agent,
                                    [result, err]() mutable noexcept
                                    { result.setError( std::move( err ) ); } );
                        }
                    } );
                return result;
            }

            namespace _private
            {
                template <class F, class... FTypes> struct ApplyCondition
                {
                    template <class S, class... Fs>
                    ApplyCondition( S&& selector, Fs&&... fs )
                        : mSelector( std::forward<F>( selector ) ),
                          mFuncs( std::forward<Fs>( fs )... )
                    {
                    }
                    F mSelector;
                    std::tuple<FTypes...> mFuncs;
                    template <class TArg, class ExecutorAgent>
                    auto operator()( ExFuture<ExecutorAgent, TArg> inputFut )
                    {
                        return condition( inputFut,
                                          std::forward<F>( mSelector ),
                                          std::forward<FTypes>(
                                              std::get<FTypes>( mFuncs ) )... );
                    }
                };
            } // namespace _private

            ///@brief version for use with operator |
            template <class F, class... FTypes>
            _private::ApplyCondition<F, FTypes...>
            condition( F&& selector, FTypes&&... functions )
            {
                return _private::ApplyCondition<F, FTypes...>(
                    std::forward<F>( selector ),
                    std::forward<FTypes>( functions )... );
            }
        } // namespace flow
    }     // namespace execution
} // namespace mel