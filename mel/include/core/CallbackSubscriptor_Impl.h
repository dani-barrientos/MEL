/*
 * SPDX-FileCopyrightText: 2005,2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
namespace mel
{
	namespace core
	{
		/**
		* @brief callback subscription functionalit
		*/
	#if VARIABLE_NUM_ARGS == VARIABLE_MAX_ARGS	
		///@cond HIDDEN_SYMBOLS
		namespace _private
		{
			template <bool> struct _CriticalSectionWrapper
			{
				CriticalSection mSC;
			};
			template <bool enabled> struct _Lock
			{
				_Lock(_CriticalSectionWrapper<enabled>& cs):mLck(cs.mSC){}
				private:
					Lock mLck;
			};
			template <> struct _CriticalSectionWrapper<false>
			{
				private:				
			};
			template <> struct _Lock<false>
			{
				_Lock(_CriticalSectionWrapper<false>& cs){}
			};
		}
		///@endcond
		template <class ThreadingPolicy, VARIABLE_ARGS >
		class CallbackSubscriptor_Base
		{
		public:		
			typedef Callback<ECallbackResult, VARIABLE_ARGS_DECL> CallbackType;

		protected:
			struct CallbackInfo
			{
				std::shared_ptr<CallbackType> cb;
				int id;  //callback id to use on unsubscriptionr when needed/neccesary
				CallbackInfo(std::shared_ptr<CallbackType> acb, int aId) :cb(acb), id(aId) {}
				CallbackInfo() :cb(nullptr), id(-1) {}
			};
			int mCurrId;
			bool mTriggering; //to detect subscription/unsubscription while triggering
			struct PendingOperation
			{
				enum class EOperation { O_SUBSCRIPTION, O_UNSUBSCRIPTION } op;
				SubscriptionEmplacement emplacement; //only for subscription
				CallbackInfo info;
			};
			std::deque <PendingOperation> mPendingOperation;

		public:
			CallbackSubscriptor_Base() :mCurrId(0), mTriggering(false){};
			typedef list< CallbackInfo > CallbackListType;
			virtual ~CallbackSubscriptor_Base()
			{
				removeCallbacks();
			}

			inline size_t getNumCallbacks() const{ return mCallbacks.size(); }
			void removeCallbacks()
			{
				_private::_Lock<mpl::isSame<ThreadingPolicy, ::mel::core::CSMultithreadPolicy>::result> lck(mSC);
				mCallbacks.clear();
			}
			template <class U>
			int subscribeCallback( U&& callback, SubscriptionEmplacement se=SE_BACK)
			{
				return subscribeCreatedCallback(new CallbackType(std::forward<U>(callback), ::mel::core::use_functor), se);
			}
			int subscribeCallback(std::function< ECallbackResult (VARIABLE_ARGS_DECL)>&& callback, SubscriptionEmplacement se=SE_BACK)
			{
				return subscribeCreatedCallback(new CallbackType(std::move(callback), ::mel::core::use_function), se);
			}
			int subscribeCallback(const std::function< ECallbackResult (VARIABLE_ARGS_DECL)>& callback, SubscriptionEmplacement se=SE_BACK)
			{
				return subscribeCreatedCallback(new CallbackType(callback, ::mel::core::use_function), se);
			}
			int subscribeCallback(std::function< ECallbackResult(VARIABLE_ARGS_DECL)>& callback, SubscriptionEmplacement se=SE_BACK)
			{
				return subscribeCreatedCallback(new CallbackType(callback, ::mel::core::use_function), se);
			}
			/**
			* std::function lacks operator ==, so need to use unsubscribecallback(int id) or use any other mpl function capabilities
			*/
			bool unsubscribeCallback(std::function< ECallbackResult(VARIABLE_ARGS_DECL)>&& callback) = delete;		
			template <class U>
			bool unsubscribeCallback( U&& callback )
			{
				_private::_Lock<mpl::isSame<ThreadingPolicy, ::mel::core::CSMultithreadPolicy>::result> lck(mSC);
				CallbackType* auxiliar = new CallbackType(::std::forward<U>(callback), ::mel::core::use_functor );
				return unsubscribeCreatedCallback( std::shared_ptr< CallbackType>( auxiliar ) );
			}
			bool unsubscribeCallback(int id)
			{
				bool result = false;
				//@todo revisar locks
				_private::_Lock<mpl::isSame<ThreadingPolicy, ::mel::core::CSMultithreadPolicy>::result> lck(mSC);
				if (mTriggering)
				{
	//				spdlog::debug("CallbackSubscriptor Callbacks are being triggered while unsubscribing!!");
					for (typename CallbackListType::iterator i = mCallbacks.begin(), j = mCallbacks.end(); i != j; ++i)
					{
						if (i->id == id)
						{
							PendingOperation po;
							po.op = PendingOperation::EOperation::O_UNSUBSCRIPTION;
							po.info = std::move(CallbackInfo(nullptr, id));
							mPendingOperation.push_back(std::move(po));
							result = true;
							break;
						}
					}
				}
				else
				{
					for (typename CallbackListType::iterator i = mCallbacks.begin(), j = mCallbacks.end(); i != j; ++i)
					{
						if (i->id == id)
						{
							mCallbacks.erase(i);
							result = true;
							break;
						}
					}
				}
				return result;			
			}
			/**
			* special case for subscription with Callback already created. Takes ownership
			*/
			int subscribeCreatedCallback(CallbackType* cb, SubscriptionEmplacement se=SE_BACK)
			{
				int result;
				_private::_Lock<mpl::isSame<ThreadingPolicy, ::mel::core::CSMultithreadPolicy>::result> lck(mSC);
					result = ++mCurrId;
				if (mTriggering)
				{
	//				spdlog::debug("CallbackSubscriptor Callbacks are being triggered while subscribing!!");
					PendingOperation po;
					po.op = PendingOperation::EOperation::O_SUBSCRIPTION;
					po.info = std::move(CallbackInfo(std::shared_ptr<CallbackType>(cb), result));
					po.emplacement = se;
					mPendingOperation.push_back(std::move(po));
				}
				else
				{
					if (se == SE_BACK) {
						mCallbacks.push_back(CallbackInfo(std::shared_ptr<CallbackType>(cb), result));
					}
					else {
						mCallbacks.push_front(CallbackInfo(std::shared_ptr<CallbackType>(cb), result));
					}
				}			
				return result;
			}
			/**
			* Unsubscribe an already existing callback
			* It looks for a callback equal to passed (cb), NOT THE POINTER
			* @param[in] cb the callback being desuscribed,
			* @return `true` if unsubscribed (`false` otherwise)
			*/
			bool unsubscribeCreatedCallback( std::shared_ptr<CallbackType> cb  )
			{
				bool result = false;
				_private::_Lock<mpl::isSame<ThreadingPolicy, ::mel::core::CSMultithreadPolicy>::result> lck(mSC);
				if (mTriggering)
				{

	//				spdlog::debug("CallbackSubscriptor Callbacks are being triggered while unsubscribing!!");
					typename CallbackListType::iterator i = mCallbacks.begin();
					while (i != mCallbacks.end())
					{
						if (*i->cb == *cb)
						{
							PendingOperation po;
							po.op = PendingOperation::EOperation::O_UNSUBSCRIPTION;
							po.info = std::move(CallbackInfo(std::shared_ptr<CallbackType>(cb), -1));
							mPendingOperation.push_back(std::move(po));
							result = true;
							break;
						}
						else
						{
							++i;
						}
					}					
				}
				else
				{
					typename CallbackListType::iterator i = mCallbacks.begin();
					while (i != mCallbacks.end())
					{
						if (*i->cb == *cb)
						{
							mCallbacks.erase(i);
							i = mCallbacks.end();
							result = true;
							break;
						}
						else
						{
							++i;
						}
					}
				}
				return result;
			}
			inline CallbackListType& getCallbacks(){return mCallbacks;}
			inline const CallbackListType& getCallbacks() const{return mCallbacks;}
		protected:
			CallbackListType mCallbacks;
			_private::_CriticalSectionWrapper<mpl::isSame<ThreadingPolicy,mel::core::CSMultithreadPolicy>::result> mSC;

			CallbackSubscriptor_Base( const CallbackSubscriptor_Base& o2 ):mTriggering(false)
			{
				_private::_Lock<mpl::isSame<ThreadingPolicy, ::mel::core::CSMultithreadPolicy>::result> lck(mSC);
				for( typename CallbackListType::iterator i = o2.mCallbacks.begin(),j = o2.mCallbacks.end(); i!=j;++i) 			
					mCallbacks.push_back( CallbackInfo((*i).cb->clone(),++mCurrId) );			
			}	
			void _applyPendingOperations()
			{
				for (auto& p : mPendingOperation)
				{
					switch (p.op)
					{
					case PendingOperation::EOperation::O_SUBSCRIPTION:
						if (p.emplacement == SE_BACK) {
							mCallbacks.push_back(p.info);
						}
						else {
							mCallbacks.push_front(p.info);
						}
						break;
					case PendingOperation::EOperation::O_UNSUBSCRIPTION:
						if (p.info.cb == nullptr) //use id for unsubscription
						{
							unsubscribeCallback(p.info.id);
						}
						else
						{
							unsubscribeCreatedCallback(p.info.cb);
						}
					}
				}
				mPendingOperation.clear();
			}

		};

		template <class ThreadingPolicy, VARIABLE_ARGS >
		class CallbackSubscriptorNotTyped: public CallbackSubscriptor_Base<ThreadingPolicy,VARIABLE_ARGS_DECL>
		{
			typedef CallbackSubscriptor_Base<ThreadingPolicy, VARIABLE_ARGS_DECL> BaseType;
		public:
			void triggerCallbacks( VARIABLE_ARGS_IMPL )
			{
				_private::_Lock<mpl::isSame<ThreadingPolicy, ::mel::core::CSMultithreadPolicy>::result> lck(BaseType::mSC);
				if (BaseType::mTriggering)
				{
	//				spdlog::debug("CallbackSubscriptor Callbacks are being triggered while  triggering again!!");
					return;
				}
				BaseType::mTriggering = true;
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				auto j = BaseType::mCallbacks.end();
				while( i != j )
				{
						if ( (*i->cb)(VARIABLE_ARGS_USE) == ECallbackResult::UNSUBSCRIBE)
					{
						i = BaseType::mCallbacks.erase( i );
					}else
					{
						++i;
					}
				}
				BaseType::mTriggering = false;
				if (!BaseType::mPendingOperation.empty())
					BaseType::_applyPendingOperations();
			}
		protected:
			CallbackSubscriptorNotTyped():BaseType(){}
			CallbackSubscriptorNotTyped( const CallbackSubscriptorNotTyped& o2 ):BaseType( o2 ){}

		};

		
		////specialization for void arguments 
		template <class ThreadingPolicy>
		class CallbackSubscriptorNotTyped<ThreadingPolicy,void> : public CallbackSubscriptor_Base<ThreadingPolicy,void>
		{
			typedef CallbackSubscriptor_Base<ThreadingPolicy,void> BaseType;
		public:

			void triggerCallbacks(  )
			{
				_private::_Lock<mpl::isSame<ThreadingPolicy, ::mel::core::CSMultithreadPolicy>::result> lck(BaseType::mSC);
				if (BaseType::mTriggering)
				{
					//spdlog::debug("CallbackSubscriptor Callbacks are being triggered while  triggering again!!");
					return;
				}
				BaseType::mTriggering = true;
				
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				auto j = BaseType::mCallbacks.end();
				while( i != j )
				{
						if ( (*i->cb)() == ECallbackResult::UNSUBSCRIBE )
					{
						i = BaseType::mCallbacks.erase( i );
					}else
					{
						++i;
					}
				}
				BaseType::mTriggering = false;
				if (!BaseType::mPendingOperation.empty())
					BaseType::_applyPendingOperations();
			}
		protected:
			CallbackSubscriptorNotTyped():BaseType(){}
			CallbackSubscriptorNotTyped( const CallbackSubscriptorNotTyped& o2 ):BaseType( o2 ){}

		};

		template < class ThreadingPolicy,VARIABLE_ARGS >
		class CallbackSubscriptor : public CallbackSubscriptorNotTyped < ThreadingPolicy,VARIABLE_ARGS_DECL>
		{
			typedef CallbackSubscriptorNotTyped <ThreadingPolicy,VARIABLE_ARGS_DECL> BaseType;
		public:
			CallbackSubscriptor():BaseType(){}
			CallbackSubscriptor( const CallbackSubscriptor& o2 ):BaseType(o2) {}

		};
		//specialization for void arguments
		template <class ThreadingPolicy >
		class CallbackSubscriptor<ThreadingPolicy,void> : public CallbackSubscriptorNotTyped <ThreadingPolicy,void>
		{
			typedef CallbackSubscriptorNotTyped<ThreadingPolicy,void> BaseType;
		public:
			CallbackSubscriptor() :BaseType() {}
			CallbackSubscriptor(const CallbackSubscriptor& o2) :BaseType(o2) {}
			int subscribeCallback( std::function< ECallbackResult()>&& callback, SubscriptionEmplacement se=SE_BACK)
			{
				return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(std::move(callback), ::mel::core::use_function), se);
			}
			int subscribeCallback(const std::function< ECallbackResult()>& callback, SubscriptionEmplacement se=SE_BACK)
			{
				return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(callback, ::mel::core::use_function), se);
			}
			int subscribeCallback(std::function< ECallbackResult()>& callback, SubscriptionEmplacement se)
			{
				return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(callback, ::mel::core::use_function), se);
			}
			template <class U>
			int subscribeCallback(U&& callback, SubscriptionEmplacement se=SE_BACK)
			{
				return BaseType::subscribeCallback(::std::forward<U>(callback), se);
			}
			/**
			* std::function lacks operator ==, so need to use unsubscribecallback(int id) or use any other mpl function capabilities
			*/
			bool unsubscribeCallback(std::function< ECallbackResult()>&& callback) = delete;		
			template <class U>
			bool unsubscribeCallback(U&& callback)
			{
				return BaseType::unsubscribeCallback(::std::forward<U>(callback) );
			}
			bool unsubscribeCallback(int id)
			{
				return BaseType::unsubscribeCallback(id);
			}
		};
		
			
	#else
		template <class ThreadingPolicy , VARIABLE_ARGS >
		class CallbackSubscriptor_Base<ThreadingPolicy , VARIABLE_ARGS_DECL,void>
		{
		public:
			typedef Callback<ECallbackResult, VARIABLE_ARGS_DECL> CallbackType;
		protected:
			struct CallbackInfo
			{
				std::shared_ptr<CallbackType> cb;
				int id;  //callback id to use on unsubscriptionr when needed/neccesary
				CallbackInfo(std::shared_ptr<CallbackType> acb, int aId) :cb(acb), id(aId) {}
				CallbackInfo() :cb(nullptr), id(-1) {}
			};
			int mCurrId;
			bool mTriggering; //to detect subscription/unsubscription while triggering
			struct PendingOperation
			{
				enum class EOperation { O_SUBSCRIPTION, O_UNSUBSCRIPTION } op;
				SubscriptionEmplacement emplacement; //only for subscription
				CallbackInfo info;
			};
			std::deque <PendingOperation> mPendingOperation;
		public:
			typedef list< CallbackInfo > CallbackListType;
			CallbackSubscriptor_Base():mCurrId(0), mTriggering(false){};
			virtual ~CallbackSubscriptor_Base()
			{
				removeCallbacks();
			}
			inline size_t getNumCallbacks() const{ return mCallbacks.size(); }
			void removeCallbacks()
			{
				_private::_Lock<mpl::isSame<ThreadingPolicy, ::mel::core::CSMultithreadPolicy>::result> lck(mSC);
				mCallbacks.clear();
			}

			template <class U>
			int subscribeCallback(U&& callback, SubscriptionEmplacement se=SE_BACK)
			{
				return subscribeCreatedCallback(new CallbackType(::std::forward<U>(callback), ::mel::core::use_functor), se);
			}

			/**
			* special case for subscription with Callback already created. Takes ownership
			*/

			int subscribeCreatedCallback( CallbackType* cb, SubscriptionEmplacement se=SE_BACK )
			{			
				int result;
				_private::_Lock<mpl::isSame<ThreadingPolicy, ::mel::core::CSMultithreadPolicy>::result> lck(mSC);
				result = ++mCurrId;
				if (mTriggering)
				{				
				//	spdlog::debug("CallbackSubscriptor Callbacks are being triggered while  triggering subscribing!!");
					PendingOperation po;
					po.op = PendingOperation::EOperation::O_SUBSCRIPTION;
					po.info = std::move(CallbackInfo(std::shared_ptr<CallbackType>(cb), result));
					po.emplacement = se;
					mPendingOperation.push_back(std::move(po));
				}
				else
				{
					if (se == SE_BACK) {
						mCallbacks.push_back(CallbackInfo(std::shared_ptr<CallbackType>(cb), result));
					}
					else {
						mCallbacks.push_front(CallbackInfo(std::shared_ptr<CallbackType>(cb), result));
					}
				}				
				return result;
			}

			template <class U>
			bool unsubscribeCallback( U&& callback )
			{
				bool result = false;
				_private::_Lock<mpl::isSame<ThreadingPolicy, ::mel::core::CSMultithreadPolicy>::result> lck(mSC);
				CallbackType* auxiliar = new CallbackType( std::forward<U>(callback), ::mel::core::use_functor );
				result = unsubscribeCreatedCallback(std::shared_ptr< CallbackType>(auxiliar));
				return result;	
			}
			bool unsubscribeCallback(int id)
			{
				bool result = false;
				_private::_Lock<mpl::isSame<ThreadingPolicy, ::mel::core::CSMultithreadPolicy>::result> lck(mSC);
						
				if (mTriggering)
				{				
					//spdlog::debug("CallbackSubscriptor Callbacks are being triggered while  triggering unsubscribing!!");
					for (typename CallbackListType::iterator i = mCallbacks.begin(), j = mCallbacks.end(); i != j; ++i)
					{
						if (i->id == id)
						{
							PendingOperation po;
							po.op = PendingOperation::EOperation::O_UNSUBSCRIPTION;
							po.info = std::move(CallbackInfo(nullptr, id));
							mPendingOperation.push_back(std::move(po));
							result = true;
							break;
						}
					}
				}
				else
				{
					for (typename CallbackListType::iterator i = mCallbacks.begin(), j = mCallbacks.end(); i != j; ++i)
					{
						if (i->id == id)
						{
							mCallbacks.erase(i);
							result = true;
							break;
						}
					}
				}				
				return result;
			}

			/**
			* unsubscribe a callback giving the already created Callback
			* 
			*/
			bool unsubscribeCreatedCallback(std::shared_ptr<CallbackType> cb  )
			{
				bool result = false;
				_private::_Lock<mpl::isSame<ThreadingPolicy, ::mel::core::CSMultithreadPolicy>::result> lck(mSC);
				if (mTriggering)
				{
					//spdlog::debug("CallbackSubscriptor Callbacks are being triggered while  triggering unsubscribing!!");
					typename CallbackListType::iterator i = mCallbacks.begin();
					while (i != mCallbacks.end())
					{
						if (*i->cb == *cb)
						{
							PendingOperation po;
							po.op = PendingOperation::EOperation::O_UNSUBSCRIPTION;
							po.info = std::move(CallbackInfo(std::shared_ptr<CallbackType>(cb), -1));
							mPendingOperation.push_back(std::move(po));
							result = true;
							break;
						}
						else
						{
							++i;
						}
					}					
				}
				else
				{
					typename CallbackListType::iterator i = mCallbacks.begin();
					while (i != mCallbacks.end())
					{
						if (*i->cb == *cb)
						{
							mCallbacks.erase(i);
							i = mCallbacks.end();
							result = true;
							break;
						}
						else
						{
							++i;
						}
					}

				}
				return result;
			}
			inline CallbackListType& getCallbacks(){return mCallbacks;}
			inline const CallbackListType& getCallbacks() const{return mCallbacks;}
		protected:
			CallbackListType mCallbacks;
			_private::_CriticalSectionWrapper<mpl::isSame<ThreadingPolicy, ::mel::core::CSMultithreadPolicy>::result> mSC;

			CallbackSubscriptor_Base( const CallbackSubscriptor_Base& o2 ):mTriggering(false)
			{
				_private::_Lock<mpl::isSame<ThreadingPolicy, ::mel::core::CSMultithreadPolicy>::result> lck(mSC);
				typename CallbackListType::const_iterator i,j;
				for( i = o2.mCallbacks.begin(),j = o2.mCallbacks.end(); i!=j;++i) 
					mCallbacks.push_back( CallbackInfo( std::shared_ptr<CallbackType>((*i).cb->clone()),++mCurrId) );
			}	
			void _applyPendingOperations()
			{
				for (auto& p : mPendingOperation)
				{
					switch (p.op)
					{
					case PendingOperation::EOperation::O_SUBSCRIPTION:
						if (p.emplacement == SE_BACK) {
							mCallbacks.push_back(p.info);
						}
						else {
							mCallbacks.push_front(p.info);
						}
						break;
					case PendingOperation::EOperation::O_UNSUBSCRIPTION:
						if (p.info.cb == nullptr) //use id for unsubscription
						{
							unsubscribeCallback(p.info.id);
						}
						else
						{
							unsubscribeCreatedCallback(p.info.cb);
						}
					}
				}
				mPendingOperation.clear();
			}
		};
		template < class ThreadingPolicy, VARIABLE_ARGS >
		class CallbackSubscriptorNotTyped<ThreadingPolicy,VARIABLE_ARGS_DECL> : public CallbackSubscriptor_Base<ThreadingPolicy, VARIABLE_ARGS_DECL>
		{
			typedef CallbackSubscriptor_Base<ThreadingPolicy, VARIABLE_ARGS_DECL> BaseType;
		public:

			void triggerCallbacks( VARIABLE_ARGS_IMPL )
			{			
				_private::_Lock<mpl::isSame<ThreadingPolicy, ::mel::core::CSMultithreadPolicy>::result> lck(BaseType::mSC);
				if (BaseType::mTriggering)
				{				
					//spdlog::debug("CallbackSubscriptor Callbacks are being triggered while  triggering again YYY!!");
					return;
				}
				BaseType::mTriggering = true;
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				while( i != BaseType::mCallbacks.end() )
				{
					if ( (*i->cb)(VARIABLE_ARGS_USE) == ECallbackResult::UNSUBSCRIBE)
					{
						i = BaseType::mCallbacks.erase( i );
					}else
					{
						++i;
					}
				}
				BaseType::mTriggering = false;
				if (!BaseType::mPendingOperation.empty())
					BaseType::_applyPendingOperations();						
			}
		protected:
			CallbackSubscriptorNotTyped():BaseType(){}
			CallbackSubscriptorNotTyped( const CallbackSubscriptorNotTyped& o2 ):BaseType( o2 ){}
		};

		template < class ThreadingPolicy, VARIABLE_ARGS >
		class CallbackSubscriptor<ThreadingPolicy,VARIABLE_ARGS_DECL,void> : public CallbackSubscriptorNotTyped<ThreadingPolicy,VARIABLE_ARGS_DECL,void>
		{
			typedef CallbackSubscriptorNotTyped<ThreadingPolicy,VARIABLE_ARGS_DECL,void> BaseType;
		public:
			CallbackSubscriptor():BaseType(){}
			CallbackSubscriptor( const CallbackSubscriptor& o2 ):BaseType( o2 ){}
			int subscribeCallback(std::function< ECallbackResult (VARIABLE_ARGS_DECL)>&& callback, SubscriptionEmplacement se=SE_BACK)
			{
				return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(std::move(callback), ::mel::core::use_function), se);
			}
			int subscribeCallback(const std::function< ECallbackResult(VARIABLE_ARGS_DECL)>& callback, SubscriptionEmplacement se=SE_BACK)
			{
				return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(callback, ::mel::core::use_function), se);
			}
			int subscribeCallback(std::function< ECallbackResult(VARIABLE_ARGS_DECL)>& callback, SubscriptionEmplacement se=SE_BACK)
			{
				return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(callback, ::mel::core::use_function), se);
			}
			
			template <class U> int subscribeCallback(U&& functor, SubscriptionEmplacement se=SE_BACK)
			{
				return BaseType::subscribeCallback(std::forward<U>(functor), se );
			}
			/**
			* std::function lacks operator ==, so need to use unsubscribecallback(int id) or use any other mpl function capabilities
			*/
			bool unsubscribeCallback(std::function< ECallbackResult(VARIABLE_ARGS_DECL)>&& callback) = delete;
			// {
			// 	//MPL_STATIC_ASSERT(true, PEPE)
			// 	//static_assert(false,"can't unsubscribe std::function<>. unsubscribe by id or use mpl function utilities")
			// 	return false; //can't unsubscribe function
			// }
			template <class U>
			bool unsubscribeCallback(U&& callback)
			{
				return BaseType::unsubscribeCallback(callback);
			}
			bool unsubscribeCallback(int id)
			{
				return BaseType::unsubscribeCallback(id);
			}
		};	
	#endif
	}
}