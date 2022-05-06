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

        #define CONDITION_SELECT_JOB(idx,size,jobs,jobarg) \
            if constexpr (size>idx) \
            { \
                using JobType = std::tuple_element_t<idx,TupleType>; \
                typedef typename decltype(selResult)::second_type ArgType; \
                auto output = launch(source.agent,[job = std::forward<JobType>(std::get<idx>(jobs))](ArgType&& arg ) mutable noexcept(std::is_nothrow_invocable<JobType,ArgType>::value)->TRet{ \
                    return job(std::forward<ArgType>(arg)); \
                },std::forward<ArgType>(jobarg)); \
                result.assign(output); \
            }else{ \
                 launch(source.agent,[result]( ) mutable noexcept {  \
                    result.setError(std::out_of_range("execution::condition. Index '" TOSTRING(idx) "' is greater than maximum case index " #size));\
                  }); \
            }
        
        /**
         * @brief Select functor to execute
         * @details callable \p selector return an unsigned int with is the index of de callable in \p jobs
         * If index is greater thatn available callables, an std::out_of_range error is set
         * @param selector callable with signature pair<size_t,T> f(TArg) wich select the given job and pass argument of type T to selected jobs
         */                
        template <class TRet,class ExecutorAgent,class TArg,class F,class ...Jobs> ExFuture<ExecutorAgent,TRet> 
            condition(ExFuture<ExecutorAgent,TArg> source, F&& selector,Jobs&&... jobs)
        {                
            typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;            
            ExFuture<ExecutorAgent,TRet> result(source.agent);            
            source.subscribeCallback(
                //need to bind de source future to not get lost and input pointing to unknown place                
                std::function<void( ValueType&)>([source,selector = std::forward<F>(selector),jobs = std::make_tuple(std::forward<Jobs>(jobs)...),result](  ValueType& input) mutable noexcept(std::is_nothrow_invocable<F,TArg>::value)
                {       
                    if ( input.isValid() )
                    {  
                        //Evaluate index
                        auto selResult = selector(input.value());  
                        size_t idx = selResult.first;
                        using TupleType = decltype(jobs);
                        constexpr size_t tsize = std::tuple_size<TupleType>::value;
                        switch(idx)
                        {
                            case 0:             
                            // {
                                
                            //     auto& job = std::get<0>(jobs);
                            //     using JobType = std::tuple_element_t<0,TupleType>;
                            //     using ArgType = typename decltype(selResult)::second_type;
                            //     //ESTOY CASI SEGURO QUE EL MOVE NO VALE; PERO NO TENGO CLARO QUE HACER
                            //     source.agent. template launch<TRet>([job = std::forward<JobType>(job)](ArgType&& arg) mutable noexcept(std::is_nothrow_invocable<JobType,ValueType&>::value)->TRet { 
                            //         return job(std::forward<ArgType>(arg));  
                            //     },std::forward<ArgType>(selResult.second),result);
                                
                            // }
                                CONDITION_SELECT_JOB(0,tsize,jobs,selResult.second)
                                break;
                            case 1:
                                CONDITION_SELECT_JOB(1,tsize,jobs,selResult.second)
                                break;
                            case 2:
                                CONDITION_SELECT_JOB(2,tsize,jobs,selResult.second)
                                break;
                             case 3:
                                CONDITION_SELECT_JOB(3,tsize,jobs,selResult.second)
                                break;
                            case 4:
                                CONDITION_SELECT_JOB(4,tsize,jobs,selResult.second)
                                break;
                            case 5:
                                CONDITION_SELECT_JOB(5,tsize,jobs,selResult.second)
                                break;
                            case 6:
                                CONDITION_SELECT_JOB(6,tsize,jobs,selResult.second)
                                break;
                            case 7:
                                CONDITION_SELECT_JOB(7,tsize,jobs,selResult.second)
                                break;
                            case 8:
                                CONDITION_SELECT_JOB(8,tsize,jobs,selResult.second)
                                break;
                            case 9:
                                CONDITION_SELECT_JOB(9,tsize,jobs,selResult.second)
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