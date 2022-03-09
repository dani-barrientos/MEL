#pragma once
//#include <execution/Continuation.h>
#include <execution/CommonDefs.h>
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
    problema, no puedo poner un default para los hints...
    y si lo resuelvo mediante un algoritmo especial? gracias a la composición, podrái tener algo como: launchHints(hints)->devuelve un Future con esos hints
    ojo, que ya no me vale el Future, voy a necesitar mi clase especial por encima, para guardar el executor, etc..

    template <class TRet,class F,class ExecutorAgent> ExFuture<ExecutorAgent,TRet> launch( Executor<ExecutorAgent> ex,F&& f,const LaunchHints& hints)
    {
        ExFuture<ExecutorAgent,TRet> result(ex);
        ex. template launch<TRet>(std::forward<F>(f),result);
        return result;
    }
    template <class TRet,class TArg,class F,class ExecutorAgent> ExFuture<ExecutorAgent,TRet> launch( Executor<ExecutorAgent> ex,F&& f,TArg&& arg)
    {
        ExFuture<ExecutorAgent,TRet> result(ex);
        ex. template launch<TRet>(std::forward<F>(f),std::forward<TArg>(arg),result);
        return result;
    }
    /**
     * @brief attach a functor to execute when input fut is complete
     * Given functor will be executed inf the input ExFuture executor. 
     */

    template <class TRet,class TArg,class ExecutorAgent,class F> ExFuture<ExecutorAgent,TRet> next(ExFuture<ExecutorAgent,TArg> fut, F&& f)
    {        
        ExFuture<ExecutorAgent,TRet> result(fut.ex);
        fut.subscribeCallback(
            std::function<::core::ECallbackResult( const typename core::FutureValue<TArg>&)>([ex = fut.ex,f = std::forward<F>(f),result](const typename core::FutureValue<TArg>& input) 
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
    /*ExFuture operator | (const ExFuture& f1,const ExFuture& f2)
    {
        return next(f1, uhm...  no tiene snetido
    }*/

}