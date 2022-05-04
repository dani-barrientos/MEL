#pragma once
/*
 * SPDX-FileCopyrightText: 2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
#include <parallelism/ThreadPool.h>
#include <execution/Executor.h>
#include <parallelism/For.h>
namespace mel
{
    namespace execution
    {   
        using ::mel::parallelism::ThreadPool;
            
        /**
         * @brief Concrete options for this type of executor
         */
        struct ThreadPoolExecutorOpts
        {
            bool independentTasks = true; //<! if true, try to make each iteration independent
            //opcion temporal, espero poder quitarla
            bool autoKill = true; //!<if true, launched tasks will be autokilled if the Runnable receives a kill signal, else, Runanble won't finish until tasks finished
        };
        /**
         * @brief Executor specialization using a ThreadPool as execution agent
         */
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
                template <class TRet,class TArg,class F> void launch( F&& f,TArg&& arg,ExFuture<ThreadPool,TRet> output) const noexcept
                {
                    if ( !mPool.expired())
                    {
                        ThreadPool::ExecutionOpts opts;
                        opts.schedPolicy = ThreadPool::SchedulingPolicy::SP_BESTFIT;
                        auto th = mPool.lock()->selectThread(opts);
                        //th->execute<TRet>(std::bind(std::forward<F>(f),std::forward<TArg>(arg)),static_cast<Future<TRet>>(output),mOpts.autoKill?Runnable::killTrue:Runnable::killFalse);
                        th->execute<TRet>(
                                [f = std::forward<F>(f),arg = std::forward<TArg>(arg)]() mutable noexcept(std::is_nothrow_invocable<F,TArg>::value) ->TRet
                                {
                                    return f(std::forward<TArg>(arg));
                                },
                                static_cast<Future<TRet>>(output)
                            ,mOpts.autoKill?Runnable::killTrue:Runnable::killFalse);                  
                    }            
                }
                template <class TRet,class F> void launch( F&& f,ExFuture<ThreadPool,TRet> output) const noexcept
                {
                    if ( !mPool.expired())
                    {
                        ThreadPool::ExecutionOpts opts;
                        opts.schedPolicy = ThreadPool::SchedulingPolicy::SP_BESTFIT;
                        auto th = mPool.lock()->selectThread(opts);
                        th->execute<TRet>(std::forward<F>(f),static_cast<Future<TRet>>(output),mOpts.autoKill?Runnable::killTrue:Runnable::killFalse);               
                    }       
                }
                template <class I, class F>	 ::mel::parallelism::Barrier loop(I&& begin, I&& end, F&& functor, int increment);
                template <class TArg,class ...FTypes> ::mel::parallelism::Barrier parallel(ExFuture<ThreadPool,TArg> fut,std::exception_ptr& excpt, FTypes&&... functions);
                template <class ReturnTuple,class TArg,class ...FTypes> ::mel::parallelism::Barrier parallel_convert(ExFuture<ThreadPool,TArg> fut,std::exception_ptr& except,ReturnTuple& result, FTypes&&... functions);
                ///@}
            private:
                std::weak_ptr<ThreadPool> mPool;      
                ThreadPoolExecutorOpts mOpts;  
        };    
        template <class I, class F>	 ::mel::parallelism::Barrier Executor<ThreadPool>::loop(I&& begin, I&& end, F&& functor, int increment)
        {
            static_assert( std::is_invocable<F,I>::value, "ThreadPoolExecutor::loop bad functor signature");
            ThreadPool::ExecutionOpts exopts;
            exopts.useCallingThread = false;
            exopts.groupTasks = !getOpts().independentTasks;
            return ::mel::parallelism::_for(getPool().lock().get(),exopts,std::forward<I>(begin),std::forward<I>(end),std::forward<F>(functor),increment );   
        }
        ///@cond HIDDEN_SYMBOLS
        namespace _private
        {
            //helper class to use ThreadPool::execute
            template <class T> class ValueWrapper
            {
                typedef typename mel::execution::ExFuture<ThreadPool,T> FutType;
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
                    std::exception_ptr error() const
                    {
                        return mFut.getValue().error();
                    }
                    operator T& () noexcept{ return mFut.getValue().value();}
                    operator const T& () const noexcept{ return mFut.getValue().value();}

                private:
                FutType mFut;
            };
        }
        ///@endcond
        template <class TArg,class ... FTypes> ::mel::parallelism::Barrier Executor<ThreadPool>::parallel(ExFuture<ThreadPool,TArg> fut,std::exception_ptr& except, FTypes&&... functions)
        {            
            ThreadPool::ExecutionOpts exopts;
            exopts.useCallingThread = false;
            exopts.groupTasks = !getOpts().independentTasks;
            return getPool().lock()->execute(exopts,except,_private::ValueWrapper<TArg>(fut),std::forward<FTypes>(functions)...);    
        }
        template <class ReturnTuple,class TArg,class ...FTypes> ::mel::parallelism::Barrier Executor<ThreadPool>::parallel_convert(ExFuture<ThreadPool,TArg> fut,std::exception_ptr& except,ReturnTuple& result, FTypes&&... functions)
        {            
            ThreadPool::ExecutionOpts exopts;
            exopts.useCallingThread = false;
            exopts.groupTasks = !getOpts().independentTasks;    
            return getPool().lock()->executeWithResult(exopts,except,result,_private::ValueWrapper<TArg>(fut),std::forward<FTypes>(functions)...);
        }
        /**
         * @brief Executor Traits for ThreadPool Executor
         */
        template <> struct ExecutorTraits<Executor<ThreadPool>>
        {
            enum {has_microthreading = true};  //support microthreading?
            enum {has_parallelism = true}; ////support true parallelism?
        };
        //! @brief alias for Executor<ThreadPool>
        typedef Executor<ThreadPool> ThreadPoolExecutor; //alias
    }
}