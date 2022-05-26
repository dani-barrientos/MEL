#pragma once
/*
 * SPDX-FileCopyrightText: 2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
#include <execution/Executor.h>
#include <tuple>
#include <preprocessor/utils.h>
namespace mel
{
    namespace execution
    {      
        namespace flow
        {
       
            /**
             * @brief Loop with independent iterations
             * @details Executes flow in a (possibly) independent way (depending on underlying executor). This flow need a first parameter being the iterator: f(I idx, auto input)
             * @param source previous result in current job
             * @param flows callables with the form ExFuture f(ExFuture)
             * @return a std::tuple with the ExFuture result of each flow (in order)
             */
            template <class TArg,class ExecutorAgent,class Flow,class I> ExFuture<ExecutorAgent,void>
                loop(ExFuture<ExecutorAgent,TArg> source, I begin, I end, Flow flow,int increment = 1)
            {
                ExFuture<ExecutorAgent,void> result(source.agent);
                //std::exception_ptr except; //@todo, uhmm, no es muy importante, porque se refiere a error en la funcion que lanza el flow..
                constexpr bool isArithIterator = mel::mpl::TypeTraits<typename std::decay<I>::type>::isArith;
                int length;
                //@todo considerar el incremento!!!
                if constexpr (isArithIterator)
                    length = (end-begin);
                else
                    length = std::distance(begin, end);
                int count = 0;                
                int nElements = (length + increment - 1) / increment;  //"manual" ceil, because ceil function fails sometimes in fast floating mode
                ::mel::parallelism::Barrier barrier(nElements);
                I i{begin};
                while(count++ < nElements)
                {
                    auto f = flow(i,source);
                    f.subscribeCallback( [barrier](const auto&) mutable
                        {
                            barrier.set();
                        }
                    );
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
                    std::function<::mel::core::ECallbackResult( const ::mel::parallelism::BarrierData&)>([result](const ::mel::parallelism::BarrierData& ) mutable
                    {                
                        result.setValue();
                        return ::mel::core::ECallbackResult::UNSUBSCRIBE; 
                    })
                );
                return result;
            }
           

            namespace _private
            {                                          
                template <class I,class F> struct ApplyLoop
                {
                    template <class T> 
                        ApplyLoop(I b, I e,T&& f,int inc):mFlow(std::forward<T>(f)),begin(std::move(b)),end(std::move(e)),increment(inc){}
                    F mFlow;
                    I begin;
                    I end;
                    int increment;
                    template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
                    {                  
                        return flow::loop(inputFut,begin,end,std::forward<F>(mFlow),increment);
                    }
                };    
            }
            
            ///@brief version for use with operator |
            template <class I,class F> flow::_private::ApplyLoop<I,F> loop(I&& begin, I&& end, F&& flow, int increment = 1)
            {
                return _private::ApplyLoop<I,F>(begin,end,std::forward<F>(flow),increment);
            }
        }
    }
}