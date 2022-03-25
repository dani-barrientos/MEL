#pragma once
#include <parallelism/ThreadPool.h>
#include <execution/Executor.h>
#include <parallelism/For.h>
namespace execution
{   
    using ::parallelism::ThreadPool;
   
    /**
     * @brief Executor specialization using a ThreadPool as execution agent
     */
     struct ThreadPoolExecutorOpts
    {
        bool independentTasks = true; //<! if true, try to make each iteration independent
        //opcion temporal, espero poder quitarla
        bool autoKill = true; //!<if true, launched tasks will be autokilled if the Runnable receives a kill signal, else, Runanble won't finish until tasks finished
    };
    template <> class Executor<ThreadPool>
    {
        public:
            Executor(std::shared_ptr<ThreadPool> pool):mPool(pool){};           
            Executor(Executor&& ex):mPool(std::move(ex.mPool)),mOpts(ex.mOpts){}
            Executor(const Executor& ex):mPool(ex.mPool),mOpts(ex.mOpts){}
            Executor& operator=(const Executor& ex){mPool = ex.mPool;mOpts = ex.mOpts;return *this;}
            Executor& operator=( Executor&& ex){mPool = std::move(ex.mPool);mOpts = ex.mOpts;return *this;}
            void setOpts(const ThreadPoolExecutorOpts& opts){ mOpts = opts;}
            const ThreadPoolExecutorOpts& getOpts(){ return mOpts;}
            std::weak_ptr<ThreadPool>& getPool(){ return mPool;}
            const std::weak_ptr<ThreadPool>& getPool() const{ return mPool;}
            ///@{ 
            //! brief mandatory interface from Executor
            template <class TRet,class TArg,class F,class ErrorType = ::core::ErrorInfo> void launch( F&& f,TArg&& arg,ExFuture<ThreadPool,TRet,ErrorType> output) const
            {
                if ( !mPool.expired())
                {
                    ThreadPool::ExecutionOpts opts;
                    opts.schedPolicy = ThreadPool::SchedulingPolicy::SP_BESTFIT;
                    auto th = mPool.lock()->selectThread(opts);
                    th->execute<TRet>(std::bind(std::forward<F>(f),std::forward<TArg>(arg)),static_cast<Future<TRet,ErrorType>>(output),mOpts.autoKill?Runnable::_killTrue:Runnable::_killFalse);
                }            
            }
            template <class TRet,class F,class ErrorType = ::core::ErrorInfo> void launch( F&& f,ExFuture<ThreadPool,TRet,ErrorType> output) const
            {
                  if ( !mPool.expired())
                {
                    ThreadPool::ExecutionOpts opts;
                    opts.schedPolicy = ThreadPool::SchedulingPolicy::SP_BESTFIT;
                    auto th = mPool.lock()->selectThread(opts);
                    th->execute<TRet>(std::forward<F>(f),static_cast<Future<TRet,ErrorType>>(output),mOpts.autoKill?Runnable::_killTrue:Runnable::_killFalse);               
                }       
            }
            template <class I, class F>	 ::parallelism::Barrier loop(I&& begin, I&& end, F&& functor, int increment);
            template <class TArg,class ...FTypes,class ErrorType = ::core::ErrorInfo> ::parallelism::Barrier bulk(ExFuture<ThreadPool,TArg,ErrorType> fut, FTypes&&... functions);
            ///@}
        private:
            std::weak_ptr<ThreadPool> mPool;      
            ThreadPoolExecutorOpts mOpts;  
    };    
    template <class I, class F>	 ::parallelism::Barrier Executor<ThreadPool>::loop(I&& begin, I&& end, F&& functor, int increment)
    {
        ThreadPool::ExecutionOpts exopts;
        exopts.useCallingThread = false;
        exopts.groupTasks = !getOpts().independentTasks;
        return ::parallelism::_for(getPool().lock().get(),exopts,std::forward<I>(begin),std::forward<I>(end),std::forward<F>(functor),increment );   
    }
    namespace _private
    {
        template <class T,class ErrorType> class ValueWrapper
        {
            typedef typename execution::ExFuture<ThreadPool,T,ErrorType> FutType;
            public:
                ValueWrapper(const FutType& fut):mFut(fut){}
                ValueWrapper(FutType&& fut):mFut(std::move(fut)){}
                ValueWrapper(ValueWrapper&& vw):mFut(std::move(vw.mFut)){}
                ValueWrapper(const ValueWrapper& vw):mFut(vw.mFut){}
                bool isValid() const{ return mFut.getValue().isValid();}
                bool isAvailable() const{ return mFut.getValue().isVailable();}
                // wrapper for std::get.  Same rules as std::Get, so bad_variant_access is thrown if not a valid value
                typename FutType::ValueType::ReturnType value()
                {
                    return mFut.getValue().value();
                }
                typename FutType::ValueType::CReturnType value() const
                {
                    return mFut.getValue().value();
                }
                const typename FutType::ErrorType& error() const
                {
                    return mFut.getValue().error();
                }
                operator T& (){ return mFut.getValue().value();}
                operator const T& () const{ return mFut.getValue().value();}

            private:
            FutType mFut;
        };
    }
    template <class TArg,class ... FTypes,class ErrorType> ::parallelism::Barrier Executor<ThreadPool>::bulk(ExFuture<ThreadPool,TArg,ErrorType> fut, FTypes&&... functions)
    {            
        ThreadPool::ExecutionOpts exopts;
        exopts.useCallingThread = false;
        exopts.groupTasks = !getOpts().independentTasks;                        
        return getPool().lock()->execute(exopts,_private::ValueWrapper<TArg,ErrorType>(fut),std::forward<FTypes>(functions)...);    


    }
//                                     
    /*template <class TArg, class I, class F>	 ExFuture<ThreadPool,TArg> loop(ExFuture<ThreadPool,TArg> fut,I&& begin, I&& end, F&& functor, int increment = 1)
    {        
        ExFuture<ThreadPool,TArg> result(fut.ex);
        typedef typename ExFuture<ThreadPool,TArg>::ValueType  ValueType;
        fut.subscribeCallback(
            std::function<::core::ECallbackResult( ValueType&)>([fut,f = std::forward<F>(functor),result,begin,end,increment](ValueType& input)  mutable
            {
                ThreadPool::ExecutionOpts exopts;
                exopts.useCallingThread = false;
                exopts.groupTasks = !fut.ex.getOpts().independentTasks;
                auto barrier = ::parallelism::_for(fut.ex.getPool().lock().get(),exopts,std::forward<I>(begin),std::forward<I>(end),std::bind(std::forward<F>(f),std::placeholders::_1,std::ref(input)),increment );
                barrier.subscribeCallback(
                    std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([result,fut](const ::parallelism::BarrierData& ) mutable
                    {
                         result.assign(std::move(fut.getValue()));
                        return ::core::ECallbackResult::UNSUBSCRIBE; 
                    }));
                    
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            }));       
        
        return result;
    };
    */
	/*template <class TArg,class ... FTypes> ExFuture<ThreadPool,TArg>  bulk(ExFuture<ThreadPool,TArg> fut, FTypes ... functions)
    {
        ExFuture<ThreadPool,TArg> result(fut.ex);
        typedef typename ExFuture<ThreadPool,TArg>::ValueType  ValueType;
        fut.subscribeCallback(            
            std::function<::core::ECallbackResult( ValueType&)>([fut,result,fs = std::make_tuple(std::forward<FTypes>(functions)...) ](ValueType& input)  mutable
            {                
                ThreadPool::ExecutionOpts exopts;
                exopts.useCallingThread = false;
                exopts.groupTasks = !fut.ex.getOpts().independentTasks;
                                
                //auto barrier = ex.getPool().lock()->execute(exopts,input,std::forward<FTypes>(functions)...);
//tengo un problema importante aquí pasando el input, que internamente el execute ahce copia

                //auto barrier = fut.ex.getPool().lock()->execute(exopts,std::ref(input),std::forward<FTypes>(std::get<FTypes>(fs))...);
                creo que la solucion sería ejecutar una lambda intermedia por cada funcion. pero al ser parametros variables...
                cómo crear tatnas lambdas como functiones?
                igual tengo que crea ralguna clase que capture el input por referencia y tenga un operator
                el ThreadPool, tal y como está, no puede resolverlo
                auto barrier = fut.ex.getPool().lock()->execute(exopts,input,std::forward<FTypes>(std::get<FTypes>(fs))...);
//                auto barrier = fut.ex.getPool().lock()->execute(exopts,mpl::createRef(input),std::forward<FTypes>(std::get<FTypes>(fs))...);
                barrier.subscribeCallback(
                    //bind fut to avoid destroying it
                    std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([fut,result](const ::parallelism::BarrierData& ) mutable
                    {
                        // @todo tengo que meditar muy bien esto
                        result.assign(std::move(fut.getValue()));
                        return ::core::ECallbackResult::UNSUBSCRIBE; 
                    }));
                    
                return ::core::ECallbackResult::UNSUBSCRIBE; 
            }));       
        
        return result;
    }
    */
   
  
    typedef Executor<ThreadPool> ThreadPoolExecutor; //alias
}