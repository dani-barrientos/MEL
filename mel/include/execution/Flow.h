#pragma once
#include <execution/Executor.h>
#include <tuple>
#include <preprocessor/utils.h>
/**
 * @file execution helper for flow control
 * 
 */

namespace mel
{
    namespace execution
    {      
        //for internal use by condition function
    #define CONDITION_SELECT_JOB(idx) \
            if constexpr (tsize>idx) \
            { \
                using FlowType = std::tuple_element_t<idx,TupleType>; \
                launch(source.agent,[source,flow = std::forward<FlowType>(std::get<idx>(flows)),result]( ArgType&& arg) mutable noexcept {  \
                    result.assign(flow.template execute<ResultType>(source,std::forward<ArgType>(arg),true)); \
                  },std::forward<ArgType>(selResult.second)); \
            }else{ \
                 launch(source.agent,[result]( ) mutable noexcept {  \
                    result.setError(std::out_of_range("execution::condition. Index '" TOSTRING(idx) "' is greater than maximum case index " TOSTRING(tsize)));\
                  }); \
            }        
        /**
         * @brief Select functor to execute
         * @details callable \p selector return an unsigned int with is the index of de callable in \p jobs
         * If index is greater thatn available callables, an std::out_of_range error is set
         * @param selector callable with signature pair<size_t,T> f(TArg) wich select the given job and pass argument of type T to selected jobs
         */                
        template <class TRet,class ExecutorAgent,class TArg,class F,class ...Flows> ExFuture<ExecutorAgent,TRet> 
            condition(ExFuture<ExecutorAgent,TArg> source, F&& selector,Flows&&... flows)
        {                
            typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;            
            typedef ExFuture<ExecutorAgent,TRet> ResultType;
            ResultType result(source.agent);            
            source.subscribeCallback(
                //need to bind de source future to not get lost and input pointing to unknown place                
                std::function<void( ValueType&)>([source,selector = std::forward<F>(selector),flows = std::make_tuple(std::forward<Flows>(flows)...),result](  ValueType& input) mutable noexcept(std::is_nothrow_invocable<F,TArg>::value)
                {       
                    if ( input.isValid() )
                    {  
                        //Evaluate index
                        auto selResult = selector(input.value());  
                        size_t idx = selResult.first;
                        using TupleType = decltype(flows);
                        typedef typename decltype(selResult)::second_type ArgType; 
                        constexpr size_t tsize = std::tuple_size<TupleType>::value;
                        switch(idx)
                        {
                            case 0:                                       
                            // if constexpr (tsize>0) 
                            // { 
                            //     using JobType = std::tuple_element_t<0,TupleType>; 
                            //     launch(source.agent,[source,flow = std::forward<JobType>(std::get<0>(flows)),result]( ArgType&& arg) mutable noexcept {  
                            //         //auto r = job(executor,std::forward<TArg>(arg)); //así sí me funciona
                            //         //result.assign(r); 
                            //         result.assign(flow.template execute<ResultType>(source,std::forward<ArgType>(arg)));
                            //         //result.assign(job(source,arg));
                            //     },std::forward<ArgType>(selResult.second));
                            // }else{ 
                            //     launch(source.agent,[result]( ) mutable noexcept {  
                            //         result.setError(std::out_of_range("execution::condition. Index '" TOSTRING(idx) "' is greater than maximum case index " TOSTRING(tsize)));
                            //     }); 
                            // }
            
                                CONDITION_SELECT_JOB(0)
                                break;
                            case 1:
                                CONDITION_SELECT_JOB(1)
                                break;
                            case 2:
                                CONDITION_SELECT_JOB(2)
                                break;
                             case 3:
                                CONDITION_SELECT_JOB(3)
                                break;
                            case 4:
                                CONDITION_SELECT_JOB(4)
                                break;
                            case 5:
                                CONDITION_SELECT_JOB(5)
                                break;
                            case 6:
                                CONDITION_SELECT_JOB(6)
                                break;
                            case 7:
                                CONDITION_SELECT_JOB(7)
                                break;
                            case 8:
                                CONDITION_SELECT_JOB(8)
                                break;
                            case 9:
                                CONDITION_SELECT_JOB(9)
                                break;
                        }                                                                          
                    }
                    else
                    {
                        //set error as task in executor
                        std::exception_ptr err = input.error();
                        launch(source.agent,[result,err]( ) mutable noexcept
                        {
                            result.setError(std::move(err));
                        });                        
                    }                                        
                })
            );
            return result;
        }  
        namespace _private
        {
            template <class TRet,class F, class ...FTypes> struct ApplyCondition
            {
                ApplyCondition(F&& selector,FTypes&&... fs):mSelector(std::forward<F>(selector)), mFuncs(std::forward<FTypes>(fs)...){}
                //ApplyCondition(const FTypes&... fs):mFuncs(fs...){}           
                F mSelector;
                std::tuple<FTypes...> mFuncs;                
                template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
                {
                    return condition<TRet>(inputFut,std::forward<F>(mSelector),std::forward<FTypes>(std::get<FTypes>(mFuncs))...);
                }
            };
        }
        
        ///@brief version for use with operator |
        template <class TRet,class F,class ...FTypes> _private::ApplyCondition<TRet,F,FTypes...> condition(F&& selector,FTypes&&... functions)
        {
            return _private::ApplyCondition<TRet,F,FTypes...>(std::forward<F>(selector),std::forward<FTypes>(functions)...);
        }
    }
}