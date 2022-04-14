 #pragma once
 /*
 * SPDX-FileCopyrightText: 2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
 #include <core/Future.h>
namespace mel
{
    namespace execution
    {
        using mel::core::Future;
        template <class ExecutorAgent> class Executor; //predeclarationj
        /**
         * @brief Extension of \ref mel::core::Future to apply to executors
         * @details Any executor function will return an ExFuture, allowing this way to chain functions
         */
        template <typename ExecutorAgent,typename ResultType>
        class ExFuture : public Future<ResultType>
        {
            public:
                ExFuture(const ExFuture& ob):Future<ResultType>(ob),agent(ob.agent){}
                ExFuture(ExFuture&& ob):Future<ResultType>(std::move(ob)),agent(std::move(ob.agent)){}
                ExFuture(Executor<ExecutorAgent> aEx):agent(aEx){}            
                ExFuture(Executor<ExecutorAgent> aEx,ResultType& val):Future<ResultType>(val),agent(aEx){}
                ExFuture(Executor<ExecutorAgent> aEx,ResultType&& val):Future<ResultType>(std::move(val)),agent(aEx){}
            
                ExFuture& operator= ( const ExFuture& f )
                {
                    Future<ResultType>::operator=( f );
                    agent = f.agent;
                    return *this;
                };
                ExFuture& operator= ( ExFuture&& f )
                {
                    Future<ResultType>::operator=( std::move(f));
                    agent = std::move(f.agent);
                    return *this;
                };
                Executor<ExecutorAgent> agent; //!< execution agent associated with this instance
            
        };
        ///@cond HIDDEN_SYMBOLS
        //for reference type
        template <typename ExecutorAgent,typename ResultType>
        class ExFuture<ExecutorAgent,ResultType&> : public Future<ResultType&>
        {
            public:
                ExFuture(const ExFuture& ob):Future<ResultType&>(ob),agent(ob.agent){}
                ExFuture(ExFuture&& ob):Future<ResultType&>(std::move(ob)),agent(std::move(ob.agent)){}
                ExFuture(Executor<ExecutorAgent> aEx):agent(aEx){}            
                ExFuture(Executor<ExecutorAgent> aEx,ResultType& val):Future<ResultType&>(val),agent(aEx){}
            
                ExFuture& operator= ( const ExFuture& f )
                {
                    Future<ResultType&>::operator=( f );
                    agent = f.agent;
                    return *this;
                };
                ExFuture& operator= ( ExFuture&& f )
                {
                    Future<ResultType&>::operator=( std::move(f));
                    agent = std::move(f.agent);
                    return *this;
                };
                Executor<ExecutorAgent> agent; //!< execution agent associated with this instance
            
        };
        //specialization for void
        template <typename ExecutorAgent>
        class ExFuture<ExecutorAgent,void> : public Future<void>
        {
            public:
                ExFuture(const ExFuture& ob):Future<void>(ob),agent(ob.agent){}
                ExFuture(ExFuture&& ob):Future<void>(std::move(ob)),agent(std::move(ob.agent)){}
                ExFuture(Executor<ExecutorAgent> aEx):agent(aEx){}            
                ExFuture(Executor<ExecutorAgent> aEx,int dummy):Future<void>(dummy),agent(aEx)
                {}            
                ExFuture& operator= ( const ExFuture& f )
                {
                    Future<void>::operator=( f );
                    agent = f.agent;
                    return *this;
                };
                ExFuture& operator= ( ExFuture&& f )
                {
                    Future<void>::operator=( std::move(f));
                    agent = std::move(f.agent);
                    return *this;
                };

                Executor<ExecutorAgent> agent; //!< execution agent associated with this instance
            
        };
    ///@endcond
    }
}
