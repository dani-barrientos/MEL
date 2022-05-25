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
            namespace _private
            {
                template <class Flow,class Predicate,class ExecutorAgent,class TArg,class FlowResult> class WhileImpl : public std::enable_shared_from_this<WhileImpl<Flow,Predicate,ExecutorAgent,TArg,FlowResult>>
                {
                    using SourceType = ExFuture<ExecutorAgent,TArg>; 
                    public:
                        template <class F,class P> static std::shared_ptr<WhileImpl<Flow,Predicate,ExecutorAgent,TArg,FlowResult>> create(F&& f,P&& p,SourceType source,FlowResult result)
                        {				
                            auto ptr = new WhileImpl<Flow,Predicate,ExecutorAgent,TArg,FlowResult>(std::forward<F>(f),std::forward<P>(p),source,result);
                            return std::shared_ptr<WhileImpl<Flow,Predicate,ExecutorAgent,TArg,FlowResult>>(ptr);
                            //return std::make_shared<WhileImpl<Flow,Predicate,ExecutorAgent,TArg,FlowResult>>(std::forward<F>(f),std::forward<P>(p),source);
                        }		
                        void execute()
                        {
                            auto fut = mFlow(mSource);				
                            auto _this = WhileImpl<Flow,Predicate,ExecutorAgent,TArg,FlowResult>::shared_from_this();
                            fut.subscribeCallback( [_this,fut](auto v)
                            {
                                _this->_callback(fut);
                            });
                        }
                        // ~WhileImpl()  //para depurar, quitarlo
                        // {
                        //     mel::text::debug("WhileImpl destructor");
                        // }		
                    private:
                        //typename RemovePointerRef<Flow>::type mFlow;
                        //typename RemovePointerRef<Predicate>::type mPred;
                        Flow 		mFlow;
                        Predicate 	mPred;
                        SourceType 	mSource;
                        FlowResult 	mResult;

                        template <class F,class P>
                        WhileImpl(F&& f,P&& p,SourceType s,FlowResult r):mFlow(std::forward<F>(f)),mPred(std::forward<P>(p)),mSource(s),mResult(r)
                        {				
                        }
                        void _callback(FlowResult res)
                        {
                            typename FlowResult::ValueType& val =  res.getValue();
                            if ( val.isValid())
                            {
                                if constexpr ( std::is_same<typename FlowResult::ValueType::ReturnType,void>::value)
                                {
                                    if ( mPred() )
                                        execute(); 
                                    else 
                                        mResult.assign(res);
                                }else
                                {
                                    if ( mPred(val.value()) )
                                        execute(); 
                                    else 
                                        mResult.assign(res);
                                }
                                
                            }else
                                mResult.setError(std::move(res.getValue().error()));				
                        }
                };
               
            }; //end _private
        
            /**
             * @brief do...while a flow
             * @details implement the pseudocode: do { r = flow() }while(predicate(r)) 
             * So, it's a sequential loop, not a parallel one where each iteration is independent
             * 
             * @param source input ExFuture from previous job
             * @param flow  flow to execute
             * @param p predicate to finish loop (returing false) or to continue (returning true)
             * @return last result of the flow 
             */
            template <class ExecutorAgent,class TArg,class Flow,class Predicate>
                auto doWhile(ExFuture<ExecutorAgent,TArg> source, Flow flow, Predicate p)
                {			 			 
                    static_assert( std::is_invocable<Flow,ExFuture<ExecutorAgent,TArg>>::value, "execution::doWhile bad flow signature");
                    typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
                    typedef std::invoke_result_t<Flow,ExFuture<ExecutorAgent,TArg>> TRet;
                    TRet result(source.agent);
                    source.subscribeCallback(
                        //need to bind de source future to not get lost and input pointing to unknown place                

                        [source,flow = std::move(flow),p = std::move(p),result](ValueType& input) mutable
                        {   
                            if ( input.isValid() )
                            {
                                auto _while = _private::WhileImpl<Flow,Predicate,ExecutorAgent,TArg,TRet>::create(std::move(flow),std::move(p),source,result);
                                _while->execute();											
                            }else
                            {
                                launch(source.agent,[result,err = std::move(input.error())]( ) mutable noexcept {  
                                    result.setError(std::move(err));
                                }); 
                            }
                        
                        }
                );
                return result;
            }            
            namespace _private
            {              
                template <class Flow,class Predicate> struct ApplyWhile
                {
                    template <class F,class P>
                    ApplyWhile(F&& flow,P&& p):mFlow(std::forward<F>(flow)),mPred(std::forward<P>(p))
                    {					
                    }				
                    // ~ApplyWhile() //PARA DEPURACION
                    // {
                    // 	mel::text::info("Destructor ApplyWhile");
                    // }
                    Flow mFlow;              
                    Predicate mPred;  
                    template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
                    {
                        return doWhile(inputFut,std::forward<Flow>(mFlow),std::forward<Predicate>(mPred));
                    }
                };
            }
            
            ///@brief version for use with operator |
            template <class Flow,class Predicate> _private::ApplyWhile<Flow,Predicate> doWhile(Flow&& flow,Predicate&& pred)
            {
                return _private::ApplyWhile<Flow,Predicate>(std::forward<Flow>(flow),std::forward<Predicate>(pred));
            }	
        }
    }
}