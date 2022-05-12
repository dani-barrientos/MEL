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
//@todo revisar bien el forwarding del flow
    #define CONDITION_SELECT_JOB(idx) \
            if constexpr (tsize>idx) \
            { \
                using FlowType = std::tuple_element_t<idx,TupleType>; \
                static_assert(      std::is_invocable<FlowType,ExFuture<ExecutorAgent,TArg>>::value, "execution::condition bad functor signature"); \
                if constexpr (std::is_nothrow_invocable<FlowType,ExFuture<ExecutorAgent,TArg>>::value) \
                    result.assign(std::get<idx>(flows)(source)); \
                else \
                { \
                    try \
                    { \
                        result.assign(std::get<idx>(flows)(source)); \
                    }catch(...) \
                    { \
                        result.setError(std::current_exception()); \
                    } \
                } \
            }else{ \
                 launch(source.agent,[result]( ) mutable noexcept {  \
                    result.setError(std::out_of_range("execution::condition. Index '" TOSTRING(idx) "' is greater than maximum case index " TOSTRING(tsize))); \
                  }); \
            }        
        /**
         * @brief Select functor to execute
         * @details Callable \p selector return an unsigned int with is the index of de callable in \p jobs
         * If index is greater than available callables, an std::out_of_range error is set
         * @param selector callable with signature 'size_t f(TArg)' which select the given job
         * @param flows variable number of callables to choose in \p selector
         */                
        template <class ExecutorAgent,class TArg,class F,class ...Flows>
         auto condition(ExFuture<ExecutorAgent,TArg> source, F&& selector,Flows&&... flows)
        {                

            typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
            typedef typename ::mel::execution::_private::GetReturn<ExFuture<ExecutorAgent,TArg>,Flows...>::type ResultTuple;
            using ResultType = std::tuple_element_t<0,ResultTuple>;
            //typedef ExFuture<ExecutorAgent,TRet> ResultType;
            ResultType result(source.agent);            
            source.subscribeCallback(
                //need to bind de source future to not get lost and input pointing to unknown place                
                 [source,selector = std::forward<F>(selector),flows = std::make_tuple(std::forward<Flows>(flows)...),result](  ValueType& input) mutable noexcept(std::is_nothrow_invocable<F,TArg>::value)
                {       
                    if ( input.isValid() )
                    {  
                        using TupleType = decltype(flows);
                        //Evaluate index
                        size_t idx = selector(input.value());  
                        constexpr size_t tsize = std::tuple_size<TupleType>::value;
                        switch(idx)
                        {
                            case 0:    
                            /*                        
                                //codigo a pelo para temas de depuracion                                       
                                if constexpr (tsize>0)
                                { 
                                    using FlowType = std::tuple_element_t<0,TupleType>;
                                    static_assert(      std::is_invocable<FlowType,ExFuture<ExecutorAgent,TArg>>::value, "execution::condition bad functor signature");
                                    if constexpr (std::is_nothrow_invocable<FlowType,ExFuture<ExecutorAgent,TArg>>::value)                                                                        
                                        result.assign(std::get<0>(flows)(source)); 
                                    else
                                    {
                                        try
                                        {
                                            result.assign(std::get<0>(flows)(source)); 
                                        }catch(...)
                                        {
                                            result.setError(std::current_exception());   
                                        }
                                    }

                                }else{ 
                                    launch(source.agent,[result]( ) mutable noexcept {  
                                        result.setError(std::out_of_range("triqui"));
                                    }); 
                                }     */
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
                }
            );
            return result;
        }  
        namespace _private
        {
            template <class F, class ...FTypes> struct ApplyCondition
            {
                template <class S,class ...Fs>
                ApplyCondition(S&& selector,Fs&&... fs):mSelector(std::forward<F>(selector)), mFuncs(std::forward<FTypes>(fs)...){}
                //ApplyCondition(const FTypes&... fs):mFuncs(fs...){}           
                F mSelector;
                std::tuple<FTypes...> mFuncs;                
                template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
                {
                    return condition(inputFut,std::forward<F>(mSelector),std::forward<FTypes>(std::get<FTypes>(mFuncs))...);
                }
            };
        }
        
        ///@brief version for use with operator |
        template <class F,class ...FTypes> _private::ApplyCondition<std::decay_t<F>,std::decay_t<FTypes>...> condition(F&& selector,FTypes&&... functions)
        {
            return _private::ApplyCondition<std::decay_t<F>,std::decay_t<FTypes>...>(std::forward<F>(selector),std::forward<FTypes>(functions)...);
        }
    }
}