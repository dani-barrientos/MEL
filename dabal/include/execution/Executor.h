#pragma once
//#include <execution/Continuation.h>
#include <execution/CommonDefs.h>
#include <type_traits>
/**
 * @brief High level execution utilities
 * @details This namespace contains class, functions..to give a consisten execution interface independet of the underliying execution system
 */
namespace execution
{        
    //template <typename ExecutorAgent,typename ResultType,typename ErrorType = ::core::ErrorInfo> class ExFuture; predeclaration
    template <class ExecutorAgent> class Executor    
    {
        //mandatory interface to imlement in specializations
        //template <class TRet,class F> void launch( F&& f,ExFuture<ExecutorAgent,TRet> output) const
        //template <class TRet,class TArg,class F> void launch( F&& f,TArg&& arg,ExFuture<ExecutorAgent,TRet> output) const;
        template <class I, class F>	 ::parallelism::Barrier loop(I&& begin, I&& end, F&& functor, int increment);
    };
    /**
     * @brief Extension of core::Future to apply to executors
     *      
     */
    template <typename ExecutorAgent,typename ResultType,typename ErrorType = ::core::ErrorInfo>
	class ExFuture : public Future<ResultType,ErrorType>
    {
        public:
            ExFuture(const ExFuture& ob):Future<ResultType,ErrorType>(ob),ex(ob.ex){}
            ExFuture(ExFuture&& ob):Future<ResultType,ErrorType>(std::move(ob)),ex(std::move(ob.ex)){}
            ExFuture(Executor<ExecutorAgent> aEx):ex(aEx){}            
            ExFuture(Executor<ExecutorAgent> aEx,ResultType& val):Future<ResultType,ErrorType>(val),ex(aEx){}
            ExFuture(Executor<ExecutorAgent> aEx,ResultType&& val):Future<ResultType,ErrorType>(std::move(val)),ex(aEx){}
		
            ExFuture& operator= ( const ExFuture& f )
            {
                Future<ResultType,ErrorType>::operator=( f );
                ex = f.ex;
                return *this;
            };
            ExFuture& operator= ( ExFuture&& f )
            {
                Future<ResultType,ErrorType>::operator=( std::move(f));
                ex = std::move(f.ex);
                return *this;
            };
            Executor<ExecutorAgent> ex;
		
    };
    //for reference type
    template <typename ExecutorAgent,typename ResultType,typename ErrorType>
	class ExFuture<ExecutorAgent,ResultType&,ErrorType> : public Future<ResultType&,ErrorType>
    {
        public:
            ExFuture(const ExFuture& ob):Future<ResultType&,ErrorType>(ob),ex(ob.ex){}
            ExFuture(ExFuture&& ob):Future<ResultType&,ErrorType>(std::move(ob)),ex(std::move(ob.ex)){}
            ExFuture(Executor<ExecutorAgent> aEx):ex(aEx){}            
            ExFuture(Executor<ExecutorAgent> aEx,ResultType& val):Future<ResultType&,ErrorType>(val),ex(aEx){}
		
            ExFuture& operator= ( const ExFuture& f )
            {
                Future<ResultType&,ErrorType>::operator=( f );
                ex = f.ex;
                return *this;
            };
            ExFuture& operator= ( ExFuture&& f )
            {
                Future<ResultType&,ErrorType>::operator=( std::move(f));
                ex = std::move(f.ex);
                return *this;
            };
            Executor<ExecutorAgent> ex;
		
    };
    //specialization for void
    template <typename ExecutorAgent,typename ErrorType>
	class ExFuture<ExecutorAgent,void,ErrorType> : public Future<void,ErrorType>
    {
        public:
            ExFuture(const ExFuture& ob):Future<void,ErrorType>(ob),ex(ob.ex){}
            ExFuture(ExFuture&& ob):Future<void,ErrorType>(std::move(ob)),ex(std::move(ob.ex)){}
            ExFuture(Executor<ExecutorAgent> aEx):ex(aEx){}            
            ExFuture(Executor<ExecutorAgent> aEx,int dummy):Future<void,ErrorType>(dummy),ex(aEx)
            {}            
            ExFuture& operator= ( const ExFuture& f )
            {
                Future<void,ErrorType>::operator=( f );
                ex = f.ex;
                return *this;
            };
            ExFuture& operator= ( ExFuture&& f )
            {
                Future<void,ErrorType>::operator=( std::move(f));
                ex = std::move(f.ex);
                return *this;
            };

            Executor<ExecutorAgent> ex;
		
    };
    namespace _private
    {
        template <class TRet> struct ApplyInmediate
        {
             template <class T>
             ApplyInmediate(T&& a):arg(std::forward<T>(a)){}
            // ApplyInmediate(TRet&& a):arg(std::move(a)){}
            // ApplyInmediate(const TRet& a):arg(a){}
            TRet arg;
            template <class TArg,class ExecutorAgent> ExFuture<ExecutorAgent,TRet> operator()(ExFuture<ExecutorAgent,TArg> fut)
            {
                return inmediate(fut,std::forward<TRet>(arg));
            }
        };
        template <class NewExecutionAgent> struct ApplyTransfer
        {
            ApplyTransfer(Executor<NewExecutionAgent>&& a):newAgent(std::forward<Executor<NewExecutionAgent>>(a)){}
            ApplyTransfer(const Executor<NewExecutionAgent>& a):newAgent(a){}
            Executor<NewExecutionAgent> newAgent;
            template <class TRet,class OldExecutionAgent> ExFuture<NewExecutionAgent,TRet> operator()(ExFuture<OldExecutionAgent,TRet> fut)
            {
                return transfer(fut,newAgent);
            }
        };
        template <class F> struct ApplyNext
        {
            ApplyNext(F&& f):mFunc(std::forward<F>(f)){}
            ApplyNext(const F& f):mFunc(f){}
            F mFunc;
            template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
            {
                return next(inputFut,std::forward<F>(mFunc));
            }
        };
        template <class I,class F> struct ApplyLoop
        {
            ApplyLoop(I b, I e,F&& f,int inc):mFunc(std::forward<F>(f)),begin(std::move(b)),end(std::move(e)),increment(inc){}
            ApplyLoop(I b, I e,const F& f,int inc):mFunc(f),begin(std::move(b)),end(std::move(e)),increment(inc){}
            F mFunc;
            I begin;
            I end;
            int increment;
            template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
            {
                return loop(inputFut,begin,end,std::forward<F>(mFunc));
            }
        };
        
        template <class ...FTypes> struct ApplyBulk
        {
            ApplyBulk(FTypes&&... fs):mFuncs(std::forward<FTypes>(fs)...){}
            ApplyBulk(const FTypes&... fs):mFuncs(fs...){}
            //ApplyBulk(I b, I e,const F& f,int inc):mFunc(f),begin(std::move(b)),end(std::move(e)),increment(inc){}
            
            std::tuple<FTypes...> mFuncs;
            template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
            {
                return bulk(inputFut,std::forward<FTypes>(std::get<FTypes>(mFuncs))...);
            }
        };
        
    }
    template <class F,class ExecutorAgent> ExFuture<ExecutorAgent,std::invoke_result_t<F>> launch( Executor<ExecutorAgent> ex,F&& f)
    {
        typedef std::invoke_result_t<F> TRet;
        ExFuture<ExecutorAgent,TRet> result(ex);
        ex. template launch<TRet>(std::forward<F>(f),result);
        return result;
    }
    template <class TArg,class F,class ExecutorAgent> ExFuture<ExecutorAgent,std::invoke_result_t<F,TArg>> launch( Executor<ExecutorAgent> ex,F&& f,TArg&& arg)
    {
        typedef std::invoke_result_t<F,TArg> TRet;
        ExFuture<ExecutorAgent,TRet> result(ex);
        ex. template launch<TRet>(std::forward<F>(f),std::forward<TArg>(arg),result);
        return result;
    }
        /**
     * @brief Start a chain of execution in given executor.     
     * 
     * @tparam ExecutorAgent 
     * @param ex 
     * @return ExFuture<ExecutorAgent,void> 
     */
    template <class ExecutorAgent> ExFuture<ExecutorAgent,void> start( Executor<ExecutorAgent> ex)
    {
        return launch(ex,[]{});
        //it's wrong, need to be executed in executor's context        
        // ExFuture<ExecutorAgent,void> result(ex,0);  //using an int to fool and construct it already as valid
        // return result;
    }   
    /**
     * @brief Produces an inmediate value in the context of the given ExFuture executor as a response to input fut completion
     * If input fut has error, the this error is forwarded to inmediate result
     */
    template <class ExecutorAgent,class TArg,class TRet> ExFuture<ExecutorAgent,TRet> inmediate( ExFuture<ExecutorAgent,TArg> fut,TRet&& arg)
    {
        typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
        ExFuture<ExecutorAgent,TRet> result(fut.ex);
        fut.subscribeCallback(
            std::function<::core::ECallbackResult( ValueType&)>([fut,result,arg = std::forward<TRet>(arg)](  ValueType& input) mutable
            {
                //do a launch to preserve Executor thread because this callback can be raised from calling thread
                //if future is already available               
                /*
    template <class TRet,class TArg,class F> void launch( F&& f,TArg&& arg,ExFuture<Runnable,TRet> output) const

source.ex. template launch<TRet>([f=std::forward<F>(f)](ExFuture<ExecutorAgent,TArg> arg)->TRet
                */
                if ( input.isValid() )
                {
                    //@todo arreglar esto para que se haga con un launch
                     /*ex. template launch<TRet>([](TRet&& arg)
                    {
                        return std::forward<TRet>(arg);
                    },std::forward<TRet>(arg),result);   */
                    // fut.ex. template launch<TRet>([](auto arg)
                    // {
                    //     //return std::forward<TRet>(arg);
                    //     return arg;
                    // },std::forward<TRet>(arg),result);  
                    result.setValue(std::forward<TRet>(arg));
                }
                else
                {
                   /* ex. template launch<TRet>([](ExFuture<ExecutorAgent,TArg> fut)
                    {
                        fut.setError(input.error());
                    },fut);  */
                    result.setError(input.error()); //@todo pasar a launch
                }
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            })
        );
        return result;  
    }

    /**
     * @brief Attach a functor to execute when input fut is complete
     * Given functor will be executed inf the input ExFuture executor. 
     */
    template <class F,class TArg,class ExecutorAgent> ExFuture<ExecutorAgent,std::invoke_result_t<F,typename ExFuture<ExecutorAgent,TArg>::ValueType&>> 
        next(ExFuture<ExecutorAgent,TArg> source, F&& f)
    {                
        typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
        typedef std::invoke_result_t<F,ValueType&> TRet;
        ExFuture<ExecutorAgent,TRet> result(source.ex);
        source.subscribeCallback(
            //need to bind de source future to not get lost and input pointing to unknown place                
            std::function<::core::ECallbackResult( ValueType&)>([source,f = std::forward<F>(f),result](  ValueType& input) mutable
            {
                //@todo menuda cacho duda: tiene sentido esto o debería ya directamente ejecutar la tarea?
                //el problema si lo hago directamente es que ese hilo queda bloqueado hasta que todo termine, y no creo que mole..
                source.ex. template launch<TRet>([f=std::forward<F>(f)](ExFuture<ExecutorAgent,TArg> arg)->TRet
                {
                    return f(arg.getValue());
                },source,result);   
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            })
        );
        return result;
    }
       
    /**
     * @brief Transfer given ExFuture to a different executor 
     * This way, continuations can be chained but executed in diferent executors
     * @code {.cpp}
     * 
     *  Create two different threads
        auto th1 = ThreadRunnable::create(true);	
		auto th2 = ThreadRunnable::create(true);
        //create executor from one of the threads
        execution::Executor<Runnable> ex(th1);		
		ex.setOpts({true,false,false});	
        //launch chain of functions starting in executor ex
      	auto fut = execution::next(execution::schedule(ex),
            [](const auto& v)->int
            {					
                text::info("Current Runnable {}",static_cast<void*>(ThreadRunnable::getCurrentRunnable()));
                text::info("Launch waiting");
                if ( ::tasking::Process::wait(5000) != tasking::Process::ESwitchResult::ESWITCH_KILL )
                {
                    //throw std::runtime_error("Error1");
                    text::info("Launch done");
                }else
                    text::info("Launch killed");
                return 4;
            }
        );
        //create another executor using the other thread..
		execution::Executor<Runnable> ex2(th2);
		//..and tranfer current execution chain to new executor
		fut = execution::transfer(fut,ex2);
        //..so chaining, continues from this new executor
		auto fut2 = execution::next(fut,[](const auto& v)
            {
                text::info("Current Runnable {}",static_cast<void*>(ThreadRunnable::getCurrentRunnable()));
                if (v.isValid())
                {
                    text::info("Next done: {}",v.value());		
                }else					
                    text::error("Next error: {}",v.error().errorMsg);		
            }
        );						
		::core::waitForFutureThread(fut2); //wait for result from current thread	
     * @endcode
     */
    template <class NewExecutorAgent,class OldExecutorAgent,class TRet> ExFuture<NewExecutorAgent,TRet> transfer(ExFuture<OldExecutorAgent,TRet> fut,Executor<NewExecutorAgent> newAgent)
    {
        ExFuture<NewExecutorAgent,TRet> result(newAgent);
        typedef typename ExFuture<OldExecutorAgent,TRet>::ValueType  ValueType;
        fut.subscribeCallback(
            std::function<::core::ECallbackResult( ValueType&)>([result](ValueType& input) mutable
            {
                result.assign(input);
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            })
        );
        return result;
    }   
    /**
     * @brief parallel (possibly, depending on executor capabilities) loop
     * @note concrete executor must provide a member loop function with neccesary interface
     * @return ExFuture<ExecutorAgent,TArg> 
     */
    template <class ExecutorAgent,class TArg, class I, class F>	 ExFuture<ExecutorAgent,TArg> loop(ExFuture<ExecutorAgent,TArg> fut,I&& begin, I&& end, F&& functor, int increment = 1)
    {
        ExFuture<ExecutorAgent,TArg> result(fut.ex);
        typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
        fut.subscribeCallback(
            std::function<::core::ECallbackResult( ValueType&)>([fut,functor = std::forward<F>(functor),result,begin = std::forward<I>(begin),end = std::forward<I>(end),increment](ValueType& input)  mutable
            {
                try
                {   
                    auto barrier  = fut.ex.loop(std::forward<I>(begin), std::forward<I>(end),
                     [f = std::forward<F>(functor),fut](I idx) mutable
                    {
                        //@todo arreglar el loop para que reciba I&&
                        //f(std::forward<I>(idx),fut.getValue());
                        f(idx,fut.getValue());
                    }
                    , increment);
                    barrier.subscribeCallback(
                        std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([result,fut](const ::parallelism::BarrierData& ) mutable
                        {
                            result.assign(std::move(fut.getValue()));
                            return ::core::ECallbackResult::UNSUBSCRIBE; 
                        }));
                }
                catch(std::runtime_error& err)
                {
                    result.setError(core::ErrorInfo(0,err.what()));
                }
                catch(...) //@todo improve error detection
                {
                    result.setError(::core::ErrorInfo(0,"ExecutorAgent::loop. Unknown error"));
                }
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            }));
        return result;
    }
    template <class ExecutorAgent,class TArg,class ...FTypes> ExFuture<ExecutorAgent,TArg> bulk(ExFuture<ExecutorAgent,TArg> fut, FTypes&&... functions)
    {
        ExFuture<ExecutorAgent,TArg> result(fut.ex);
        typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
        fut.subscribeCallback(            
            std::function<::core::ECallbackResult( ValueType&)>([fut,result,fs = std::make_tuple(std::forward<FTypes>(functions)... )](ValueType& input)  mutable
            {
                auto barrier  = fut.ex.bulk(fut,std::forward<FTypes>(std::get<FTypes>(fs))...);
                barrier.subscribeCallback(
                    std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([fut,result](const ::parallelism::BarrierData& ) mutable
                    {
                        // @todo tengo que meditar muy bien esto
                        result.assign(std::move(fut.getValue()));
                        return ::core::ECallbackResult::UNSUBSCRIBE; 
                    }));
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            }
        ));
        return result;
    }
    //@brief version for use with operator |
    template <class TRet> _private::ApplyInmediate<TRet> inmediate( TRet&& arg)
    {

        return _private::ApplyInmediate<TRet>(std::forward<TRet>(arg));
    }
    //@brief version for use with operator |
    template <class NewExecutionAgent> _private::ApplyTransfer<NewExecutionAgent> transfer(Executor<NewExecutionAgent> newAgent)
    {
        return _private::ApplyTransfer<NewExecutionAgent>(newAgent);
    }
    
    //@brief version for use with operator |
    template <class F> _private::ApplyNext<F> next(F&& f)
    {
        return _private::ApplyNext<F>(std::forward<F>(f));
    }
    template <class I,class F> _private::ApplyLoop<I,F> loop(I&& begin, I&& end, F&& functor, int increment = 1)
    {
        return _private::ApplyLoop<I,F>(begin,end,std::forward<F>(functor),increment);
    }
    template <class ...FTypes> _private::ApplyBulk<FTypes...> bulk(FTypes&&... functions)
    {
        return _private::ApplyBulk<FTypes...>(std::forward<FTypes>(functions)...);
    }
    /**
     * @brief overload operator | for chaining
    */
    template <class ExecutorAgent,class TRet1,class U> auto operator | (ExFuture<ExecutorAgent,TRet1> input,U&& u)
    {
        return u(input);

    }            

}