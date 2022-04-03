 #pragma once
 #include <core/Future.h>
 namespace execution
 {
     using core::Future;
     template <class ExecutorAgent> class Executor; //predeclarationj
    /**
     * @brief Extension of \ref core::Future to apply to executors
     * Any executor function will return an ExFuture, allowing this way to chain functions
     */
    template <typename ExecutorAgent,typename ResultType>
	class ExFuture : public Future<ResultType>
    {
        public:
            ExFuture(const ExFuture& ob):Future<ResultType>(ob),ex(ob.ex){}
            ExFuture(ExFuture&& ob):Future<ResultType>(std::move(ob)),ex(std::move(ob.ex)){}
            ExFuture(Executor<ExecutorAgent> aEx):ex(aEx){}            
            ExFuture(Executor<ExecutorAgent> aEx,ResultType& val):Future<ResultType>(val),ex(aEx){}
            ExFuture(Executor<ExecutorAgent> aEx,ResultType&& val):Future<ResultType>(std::move(val)),ex(aEx){}
		
            ExFuture& operator= ( const ExFuture& f )
            {
                Future<ResultType>::operator=( f );
                ex = f.ex;
                return *this;
            };
            ExFuture& operator= ( ExFuture&& f )
            {
                Future<ResultType>::operator=( std::move(f));
                ex = std::move(f.ex);
                return *this;
            };
            Executor<ExecutorAgent> ex;
		
    };
    ///@cond HIDDEN_SYMBOLS
    //for reference type
    template <typename ExecutorAgent,typename ResultType>
	class ExFuture<ExecutorAgent,ResultType&> : public Future<ResultType&>
    {
        public:
            ExFuture(const ExFuture& ob):Future<ResultType&>(ob),ex(ob.ex){}
            ExFuture(ExFuture&& ob):Future<ResultType&>(std::move(ob)),ex(std::move(ob.ex)){}
            ExFuture(Executor<ExecutorAgent> aEx):ex(aEx){}            
            ExFuture(Executor<ExecutorAgent> aEx,ResultType& val):Future<ResultType&>(val),ex(aEx){}
		
            ExFuture& operator= ( const ExFuture& f )
            {
                Future<ResultType&>::operator=( f );
                ex = f.ex;
                return *this;
            };
            ExFuture& operator= ( ExFuture&& f )
            {
                Future<ResultType&>::operator=( std::move(f));
                ex = std::move(f.ex);
                return *this;
            };
            Executor<ExecutorAgent> ex;
		
    };
    //specialization for void
    template <typename ExecutorAgent>
	class ExFuture<ExecutorAgent,void> : public Future<void>
    {
        public:
            ExFuture(const ExFuture& ob):Future<void>(ob),ex(ob.ex){}
            ExFuture(ExFuture&& ob):Future<void>(std::move(ob)),ex(std::move(ob.ex)){}
            ExFuture(Executor<ExecutorAgent> aEx):ex(aEx){}            
            ExFuture(Executor<ExecutorAgent> aEx,int dummy):Future<void>(dummy),ex(aEx)
            {}            
            ExFuture& operator= ( const ExFuture& f )
            {
                Future<void>::operator=( f );
                ex = f.ex;
                return *this;
            };
            ExFuture& operator= ( ExFuture&& f )
            {
                Future<void>::operator=( std::move(f));
                ex = std::move(f.ex);
                return *this;
            };

            Executor<ExecutorAgent> ex;
		
    };
    ///@endcond
 }