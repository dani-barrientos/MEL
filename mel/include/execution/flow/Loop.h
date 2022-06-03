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

            /**
             * @brief Loop with independent iterations
             * @details Executes flow in a (possibly) independent way (depending
             * on underlying executor). This flow need a first parameter being
             * the iterator: f(I idx, auto input)
             * @param source previous result in current job
             * @param getIteratorsFunc Callable returning an std::array<I,2>
             * with begin and end iterators to loop in
             * @param flows callables with the form ExFuture f(ExFuture)
             * @return a std::tuple with the ExFuture result of each flow (in
             * order)
             */
            template <class TArg, class ExecutorAgent, class Flow, class I>
            ExFuture<ExecutorAgent, void>
            loop( ExFuture<ExecutorAgent, TArg> source, I getIteratorsFunc,
                  Flow flow, int increment = 1 )
            {
                ExFuture<ExecutorAgent, void> result( source.agent );
                // std::exception_ptr except; //@todo, uhmm, no es muy
                // importante, porque se refiere a error en la funcion que lanza
                // el flow.
                int length;
                //@todo pending to pass input result to this function. The
                // problem I see is about error management: is there is an
                // error, this function can't be called, so the flows won't be
                // able to
                // catch the error
                auto iterators = getIteratorsFunc();
                using IteratorType = decltype( iterators[0] );
                constexpr bool isArithIterator = mel::mpl::TypeTraits<
                    typename std::decay<IteratorType>::type>::isArith;
                if constexpr ( isArithIterator )
                    length = ( iterators[1] - iterators[0] );
                else
                    length = std::distance( iterators[0], iterators[1] );
                int count = 0;
                int nElements =
                    ( length + increment - 1 ) /
                    increment; //"manual" ceil, because ceil function fails
                               // sometimes in fast floating mode
                ::mel::parallelism::Barrier barrier( nElements );
                IteratorType i{ iterators[0] };
                while ( count++ < nElements )
                {
                    auto f = flow( i, source );
                    f.subscribeCallback( [barrier]( const auto& ) mutable
                                         { barrier.set(); } );
                    i += increment;
                }
                // for(auto i = begin;i != end ;i+=increment)
                // {
                //     auto f = flow(i,source);
                //     f.subscribeCallback( [barrier](const auto&) mutable
                //         {
                //             barrier.set();
                //         }
                //     );

                // }
                barrier.subscribeCallback(
                    std::function<::mel::core::ECallbackResult(
                        const ::mel::parallelism::BarrierData& )>(
                        [result](
                            const ::mel::parallelism::BarrierData& ) mutable
                        {
                            result.setValue();
                            return ::mel::core::ECallbackResult::UNSUBSCRIBE;
                        } ) );
                return result;
            }

            namespace _private
            {
                template <class I, class F> struct ApplyLoop
                {
                    template <class U, class T>
                    ApplyLoop( U&& its, T&& f, int inc )
                        : mGetIts( std::forward<U>( its ) ),
                          mFlow( std::forward<T>( f ) ), increment( inc )
                    {
                    }
                    I mGetIts;
                    F mFlow;
                    int increment;
                    template <class TArg, class ExecutorAgent>
                    auto operator()( ExFuture<ExecutorAgent, TArg> inputFut )
                    {
                        return flow::loop( inputFut, std::forward<I>( mGetIts ),
                                           std::forward<F>( mFlow ),
                                           increment );
                    }
                };
            } // namespace _private

            ///@brief version for use with operator |
            template <class I, class F>
            flow::_private::ApplyLoop<I, F> loop( I&& getItFunc, F&& flow,
                                                  int increment = 1 )
            {
                return _private::ApplyLoop<I, F>( std::forward<I>( getItFunc ),
                                                  std::forward<F>( flow ),
                                                  increment );
            }
        } // namespace flow
    }     // namespace execution
} // namespace mel