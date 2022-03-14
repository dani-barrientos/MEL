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
    
    template <class ExecutorAgent> class Executor    
    {
        /*//mandatory interface to imlement in specializations
        template <class TRet,class TArg,class F> Continuation<TRet,TArg,Executor<Runnable>> launch( F&& f,const typename Continuation<TRet,TArg,Executor<Runnable>>::ArgType& arg);
        template <class TRet,class TArg,class F> Continuation<TRet,TArg,Executor<Runnable>> launch( F&& f,TArg&& arg);
        template <class TRet,class F> Continuation<TRet,void,Executor<Runnable>> launch( F&& f); //@todo resolver el pasar parámetro a la priemra. Esto tiene varios problemas que igual no merece la pena
        template <class I, class F>	 ::parallelism::Barrier loop(I&& begin, I&& end, F&& functor,const LoopHints& hints, int increment = 1);*/
    };

    // template <class TRet,class TArg,class F,class ExAgent> Continuation<TRet,TArg,Executor<ExAgent>> launch( Executor<ExAgent> ex, F&& f,const typename Continuation<TRet,TArg,Executor<Runnable>>::ArgType& arg);
    // template <class TRet,class TArg,class F,class ExAgent> Continuation<TRet,TArg,Executor<ExAgent>> launch( Executor<ExAgent> ex,F&& f,TArg&& arg);
    // template <class TRet,class F,class ExAgent> Continuation<TRet,void,Executor<ExAgent>> launch( Executor<ExAgent> ex,F&& f); //@todo resolver el pasar parámetro a la priemra. Esto tiene varios problemas que igual no merece la pena
    // template <class I, class F,class ExAgent>	 ::parallelism::Barrier loop(Executor<ExAgent> ex,I&& begin, I&& end, F&& functor,const LoopHints& hints, int increment = 1);
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
    /**
     * @brief Start a chain of execution in given executor.     
     * 
     * @tparam ExecutorAgent 
     * @param ex 
     * @return ExFuture<ExecutorAgent,void> 
     */
    template <class ExecutorAgent> ExFuture<ExecutorAgent,void> start( Executor<ExecutorAgent> ex)
    {
        ExFuture<ExecutorAgent,void> result(ex); 
        //@todo sustituir por construccion con valor cuando esté disponiel
        result.setValue();
        return result;
    }
   
    template <class F,class ExecutorAgent> ExFuture<ExecutorAgent,std::invoke_result_t<F>> launch( Executor<ExecutorAgent> ex,F&& f)
    {
        typedef std::invoke_result_t<F> TRet;
        ExFuture<ExecutorAgent,TRet> result(ex);
        ex. template launch<TRet>(std::forward<F>(f),result);
        return result;
    }
    template <class TArg,class F,class ExecutorAgent> ExFuture<ExecutorAgent,std::invoke_result_t<F,TArg&&>> launch( Executor<ExecutorAgent> ex,F&& f,TArg&& arg)
    {
        typedef std::invoke_result_t<F,TArg> TRet;
        ExFuture<ExecutorAgent,TRet> result(ex);
        ex. template launch<TRet>(std::forward<F>(f),std::forward<TArg>(arg),result);
        return result;
    }
     /**
     * @brief Produces an inmediate value in the context of the given ExFuture executor
     */
    template <class ExecutorAgent,class TArg,class TRet> ExFuture<ExecutorAgent,TRet> inmediate( ExFuture<ExecutorAgent,TArg> fut,TRet&& arg)
    {

        return next(fut,[arg = std::forward<TRet>(arg)](const auto& v)
            {
                return arg;
            }
        );       
    }
    /**
     * @brief Attach a functor to execute when input fut is complete
     * Given functor will be executed inf the input ExFuture executor. 
     */
    template <class F,class TArg,class ExecutorAgent> ExFuture<ExecutorAgent,std::invoke_result_t<F,typename ExFuture<ExecutorAgent,TArg>::ValueType&>> 
        next(ExFuture<ExecutorAgent,TArg> fut, F&& f)
    {                
        typedef std::invoke_result_t<F,typename ExFuture<ExecutorAgent,TArg>::ValueType&> TRet;
        ExFuture<ExecutorAgent,TRet> result(fut.ex);
        fut.subscribeCallback(
            std::function<::core::ECallbackResult( typename core::FutureValue<TArg>&)>([ex = fut.ex,f = std::forward<F>(f),result]( typename core::FutureValue<TArg>& input) 
            {
                ex. template launch<TRet>(f,input,result);        
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            })
        );
        //spdlog::debug("next. Not available");
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
        fut.subscribeCallback(
            std::function<::core::ECallbackResult( const typename core::FutureValue<TRet>&)>([result](const typename core::FutureValue<TRet>& input) mutable
            {
                result.assign(input);
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            })
        );
        return result;
    }
    /**
     * @brief overload meaning same as next, for ease of use
    */
//    estoy pijo, en la propuesta el | no es el then, es solo una forma de pasar resutlado
//     template <class ExecutorAgent,class TRet1,class TRet2,class F> ExFuture<ExecutorAgent,TRet2> operator | (const ExFuture<ExecutorAgent,TRet1>& f1,F&& f)
//     {
//         return next(f1,std::forward(f));
//     }
    

}