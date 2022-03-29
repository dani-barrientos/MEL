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
            template <class TArg,class ...FTypes,class ErrorType = ::core::ErrorInfo> ::parallelism::Barrier parallel(ExFuture<ThreadPool,TArg,ErrorType> fut, FTypes&&... functions);
            template <class ReturnTuple,class TArg,class ...FTypes,class ErrorType = ::core::ErrorInfo> ::parallelism::Barrier parallel_convert(ExFuture<ThreadPool,TArg,ErrorType> fut,ReturnTuple& result, FTypes&&... functions);
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
    template <class TArg,class ... FTypes,class ErrorType> ::parallelism::Barrier Executor<ThreadPool>::parallel(ExFuture<ThreadPool,TArg,ErrorType> fut, FTypes&&... functions)
    {            
        ThreadPool::ExecutionOpts exopts;
        exopts.useCallingThread = false;
        exopts.groupTasks = !getOpts().independentTasks;                        
        return getPool().lock()->execute(exopts,_private::ValueWrapper<TArg,ErrorType>(fut),std::forward<FTypes>(functions)...);    
    }
    template <class ReturnTuple,class TArg,class ...FTypes,class ErrorType> ::parallelism::Barrier Executor<ThreadPool>::parallel_convert(ExFuture<ThreadPool,TArg,ErrorType> fut,ReturnTuple& result, FTypes&&... functions)
    {            
        ThreadPool::ExecutionOpts exopts;
        exopts.useCallingThread = false;
        exopts.groupTasks = !getOpts().independentTasks;    
        return getPool().lock()->executeWithResult(exopts,result,_private::ValueWrapper<TArg,ErrorType>(fut),std::forward<FTypes>(functions)...);
    }
    typedef Executor<ThreadPool> ThreadPoolExecutor; //alias
}