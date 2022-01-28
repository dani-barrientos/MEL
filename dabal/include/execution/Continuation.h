#pragma once
namespace execution
{
    /**
     * @details A Continuation is a wrapper for a task executed by a executor, allowing to chain another execution and check the result
     * Example:
     * @code {.language-id}
     *  execution::Executor<...whatever execution agent...> ex(th1);
			auto cont = ex.launch<int>(  
				[](const auto& v)
				{					
					if ( v.isValid() )					
						spdlog::debug("Value ");
					else
						spdlog::error("Error = {}",v.error().errorMsg);
					::tasking::Process::wait(4000);
					
					return 5;
				}
			).next<void>([](const auto& v)
			{
				if ( v.isValid() )
				{
					spdlog::debug("Value = {}",v.value());
				}
				else
				{
					spdlog::error("Error = {}",v.error().errorMsg);
				}
			})
     * @endcode
     * 
     * @tparam TRet 
     * @tparam TArg 
     * @tparam ExecutorType 
     */
    template <class TRet,class TArg,class ExecutorType> class Continuation;
    ///@cond HIDDEN_SYMBOLS
    namespace _private
    {
        template <class TArg> class ContinuationDataBase
        {
            public: //@todo meterlo privado
                virtual void start(const typename Future<TArg>::ValueType& v) = 0;
        };

        template <class TRet,class TArg,class ExecutorType> class ContinuationData final :
            public ContinuationDataBase<TArg>,
            public enable_shared_from_this<ContinuationData<TRet,TArg,ExecutorType>>
        {
            typedef enable_shared_from_this<ContinuationData<TRet,TArg,ExecutorType>> Base;
            public:
                typedef Future<TRet> FutureType;
                typedef typename Future<TArg>::ValueType ArgType;
                template <class F> ContinuationData(ExecutorType ex,F&& f ):mState(EState::NONE), mExecutor(std::move(ex)),mFunct(std::forward<F>(f)),mNext(nullptr)
                {			
                }
                template <class NewRet,class F> Continuation<NewRet,TRet,ExecutorType> next(F&& f);		
                void start(const ArgType& arg) override
                {
                    mExecutor.template _execute<TRet>(std::bind(mFunct,arg),mResult);
                    mResult.subscribeCallback(
                        std::function<::core::ECallbackResult( const typename decltype(mResult)::ValueType&)>([_this = Base::shared_from_this()](const typename decltype(mResult)::ValueType& input) 
                        {
                            ::core::Lock lck(_this->mCS);
                            _this->mState = EState::DONE;
                            if ( _this->mNext )
                                _this->mNext->start(input);
                            return ::core::ECallbackResult::UNSUBSCRIBE; 
                        })
                    );
                }		
                FutureType& getResult() { return mResult;}
            private:	
                enum class EState :uint8_t{NONE,DOING,DONE};
                CriticalSection mCS;
                ExecutorType 	mExecutor;
                EState 			mState;		
                FutureType mResult;
                std::shared_ptr<ContinuationDataBase<TRet>> mNext;
                std::function<TRet(ArgType)> mFunct;
            
        };
    }
    ///@endcond 
    template <class TRet,class TArg,class ExecutorType> class Continuation final
    {
        friend ExecutorType;
        public:
            typedef _private::ContinuationData<TRet,TArg,ExecutorType> DataType;
            typedef typename DataType::FutureType FutureType;
            typedef typename DataType::ArgType ArgType;
            template <class F> Continuation(ExecutorType ex,F&& f ):mData(make_shared<DataType>(ex,std::forward<F>(f))){}		
            template <class NewRet,class F> Continuation<NewRet,TRet,ExecutorType> next(F&& f);           
            Continuation(shared_ptr<DataType> data):mData(data){}  //@todo debe ser privado pero algo no me va bien con el friend
            Continuation(Continuation&& cont):mData(std::move(cont.mData)){}
            Continuation(const Continuation& cont):mData(cont.mData){}
            FutureType& getResult() { return mData->getResult();}
        private:
            void _start(const typename DataType::ArgType& arg){ if(mData) mData->start(arg);}
            shared_ptr<DataType> mData;

    };
    template <class TRet,class TArg,class ExecutorType> template <class NewRet,class F> Continuation<NewRet,TRet,ExecutorType> Continuation<TRet,TArg,ExecutorType>::next(F&& f)
    {
        return mData->template next<NewRet>(std::forward<F>(f));
    }
    namespace _private
    {
        template <class TRet,class TArg,class ExecutorType> template <class NewRet,class F> Continuation<NewRet,TRet,ExecutorType> ContinuationData<TRet,TArg,ExecutorType>::next(F&& f)
        {
            ::core::Lock lck(mCS);
            if ( mState== EState::DONE)
            {
                //spdlog::debug("next. Straight execution");
                //straight execution 
                return mExecutor.template launch<NewRet,TRet>(f,mResult.getValue());
            }else
            {
                //spdlog::debug("next. Not available");
                mNext = make_shared<typename Continuation<NewRet,TRet,ExecutorType>::DataType>(mExecutor,std::forward<F>(f));
                return Continuation<NewRet,TRet,ExecutorType>(std::static_pointer_cast<ContinuationData<NewRet,TRet,ExecutorType>>(mNext)); 
            }
        }	
    }
}