namespace core
{
	/**
	* base class for classes with callback subscription functionality
	* template arguments:
	* @tparam postSubscription[in] If true, then TRet(which must be bool) is interpreted as a value for unsubscription after
	* executing
	*
	* @todo now very simple and in pañales, but useful to save time: allow container selection, allow pointer-to-CallbackListType or object selection 
	* @todo VERSION CHAPUCERA DEL MULTITHREADED A LA ESPERA DE TENER COMPLETAS LAS HERRAMIENTAS NECESARIAS

	*/
#if VARIABLE_NUM_ARGS == VARIABLE_MAX_ARGS
	template <bool multithreaded, class TRet, VARIABLE_ARGS >
	class CallbackSubscriptor_Base
	{
	public:
		typedef Callback<TRet, VARIABLE_ARGS_DECL> CallbackType;

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
		//typedef list< CallbackType* > CallbackListType;
		typedef list< CallbackInfo > CallbackListType;
		virtual ~CallbackSubscriptor_Base()
		{
			removeCallbacks();
		}

		inline size_t getNumCallbacks() const{ return mCallbacks.size(); }
		void removeCallbacks()
		{
			if (multithreaded)
			{
				Lock lck( mSC );			
				mCallbacks.clear();
			}
			else
			{
				mCallbacks.clear();
			}
		}
		template <class U>
		int subscribeCallback( U&& callback, SubscriptionEmplacement se=SE_BACK)
		{
			return subscribeCreatedCallback(new CallbackType(std::forward<U>(callback), ::core::use_functor), se);
		}
		int subscribeCallback(std::function< TRet(VARIABLE_ARGS_DECL)>&& callback, SubscriptionEmplacement se=SE_BACK)
		{
			return subscribeCreatedCallback(new CallbackType(std::move(callback), ::core::use_function), se);
		}
		int subscribeCallback(const std::function< TRet(VARIABLE_ARGS_DECL)>& callback, SubscriptionEmplacement se=SE_BACK)
		{
			return subscribeCreatedCallback(new CallbackType(callback, ::core::use_function), se);
		}
		/*int subscribeCallback(std::function< TRet(VARIABLE_ARGS_DECL)>& callback, SubscriptionEmplacement se=SE_BACK)
		{
			return subscribeCreatedCallback(new CallbackType(callback, ::core::use_function), se);
		}*/
		//removed because unimplemented, so linker error
		bool unsubscribeCallback(std::function< TRet(VARIABLE_ARGS_DECL)>&& callback)
		{
			return false; //Can't unsubscribe function<>
		}
		
		template <class U>
		bool unsubscribeCallback( U&& callback )
		{
			if (multithreaded)
			{
				Lock lck( mSC );
				CallbackType* auxiliar = new CallbackType(::std::forward<U>(callback), ::core::use_functor );
				return unsubscribeCreatedCallback( std::shared_ptr< CallbackType>( auxiliar ) );
			}
			else
			{
				CallbackType* auxiliar = new CallbackType(::std::forward<U>(callback), ::core::use_functor );
				return unsubscribeCreatedCallback(std::shared_ptr< CallbackType>(auxiliar));
			}
		}
		bool unsubscribeCallback(int id)
		{
			bool result = false;
			//ESTÁN MAL LOS LOCKS
			if (multithreaded)
			{

				Lock lck(mSC);
				if (mTriggering)
				{
					//::logging::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while unsubscribing!!");					
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
			else
			{
				if (mTriggering)
				{
					//::logging::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while subscribing!!");
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
		}
		/**
		* special case for subscription with Callback already created. Takes ownership
		*/
		int subscribeCreatedCallback(CallbackType* cb, SubscriptionEmplacement se=SE_BACK)
		{
			int result;
			if (multithreaded)
			{
				Lock lck(mSC);
				result = ++mCurrId;
				if (mTriggering)
				{
					//::logging::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while subscribing!!");
					PendingOperation po;
					po.op = PendingOperation::EOperation::O_SUBSCRIPTION;
					po.info = std::move(CallbackInfo(cb, result));
					po.emplacement = se;
					mPendingOperation.push_back(std::move(po));
				}
				else
				{
					if (se == SE_BACK) {
						mCallbacks.push_back(CallbackInfo(cb, result));
					}
					else {
						mCallbacks.push_front(CallbackInfo(cb, result));
					}
				}
			}
			else {
				result = ++mCurrId;
				if (mTriggering)
				{
					//::logging::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while subscribing!!");
					PendingOperation po;
					po.op = PendingOperation::EOperation::O_SUBSCRIPTION;
					po.info = std::move(CallbackInfo(cb, result));
					po.emplacement = se;
					mPendingOperation.push_back(std::move(po));
					
				}
				else
				{
					if (se == SE_BACK) {
						mCallbacks.push_back(CallbackInfo(cb, result));
					}
					else {
						mCallbacks.push_front(CallbackInfo(cb, result));
					}
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
			if (multithreaded)
			{
				Lock lck( mSC );
				if (mTriggering)
				{

					//::logging::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while unsubscribing!!");
					typename CallbackListType::iterator i = mCallbacks.begin();
					while (i != mCallbacks.end())
					{
						if (*i->cb == *cb)
						{
							PendingOperation po;
							po.op = PendingOperation::EOperation::O_UNSUBSCRIPTION;
							po.info = std::move(CallbackInfo(cb, -1));
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
			}
			else
			{
				if (mTriggering)
				{
					//::logging::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while unsubscribing!!");
					typename CallbackListType::iterator i = mCallbacks.begin();
					while (i != mCallbacks.end())
					{
						if (*i->cb == *cb)
						{
							PendingOperation po;
							po.op = PendingOperation::EOperation::O_UNSUBSCRIPTION;
							po.info = std::move(CallbackInfo(cb, -1));
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
							return result;
						}
						else
						{
							++i;
						}
					}
				}
			}
			return result;
		}

	protected:
		CallbackListType mCallbacks;
		CriticalSection mSC;

		CallbackSubscriptor_Base( const CallbackSubscriptor_Base& o2 ):mTriggering(false)
		{
			if (multithreaded)
			{
				Lock lck( mSC );
				for( typename CallbackListType::iterator i = o2.mCallbacks.begin(),j = o2.mCallbacks.end(); i!=j;++i) 			
					mCallbacks.push_back( CallbackInfo((*i).cb->clone(),++mCurrId) );
			}
			else
			{
				for( typename CallbackListType::iterator i = o2.mCallbacks.begin(),j = o2.mCallbacks.end(); i!=j;++i) 			
					mCallbacks.push_back( CallbackInfo((*i).cb->clone(),++mCurrId) );
			}			
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

	template <bool multithreaded,bool postSubscription,class TRet, VARIABLE_ARGS >
	class CallbackSubscriptorNotTyped : public CallbackSubscriptor_Base<multithreaded,TRet,VARIABLE_ARGS_DECL>
	{
		typedef CallbackSubscriptor_Base<multithreaded, TRet,VARIABLE_ARGS_DECL> BaseType;
	public:

		void triggerCallbacks( VARIABLE_ARGS_IMPL )
		{
			if (BaseType::mTriggering)
			{
				//::logging::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while triggering again!!");
				return;
			}
			BaseType::mTriggering = true;
			if (multithreaded)
			{

				Lock lck( BaseType::mSC );
				for( typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin(),j = BaseType::mCallbacks.end(); i != j; ++i )
					(*i->cb)(VARIABLE_ARGS_USE);
				BaseType::mTriggering = false;
				if ( !BaseType::mPendingOperation.empty() )
					BaseType::_applyPendingOperations();
			}
			else
			{
				for( typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin(), j = BaseType::mCallbacks.end(); i != j; ++i )
					(*i->cb)(VARIABLE_ARGS_USE);
				BaseType::mTriggering = false;
				if (!BaseType::mPendingOperation.empty())
					BaseType::_applyPendingOperations();
			}			
			
		}
	protected:
		CallbackSubscriptorNotTyped():BaseType(){}
		CallbackSubscriptorNotTyped( const CallbackSubscriptorNotTyped& o2 ):BaseType( o2 ){}
	};
	//specialization for postSubscription == true
	template <bool multithreaded,class TRet, VARIABLE_ARGS_NODEFAULT >
	class CallbackSubscriptorNotTyped<multithreaded,true,TRet,VARIABLE_ARGS_DECL> : public CallbackSubscriptor_Base<multithreaded,TRet,VARIABLE_ARGS_DECL>
	{
		typedef CallbackSubscriptor_Base<multithreaded, TRet,VARIABLE_ARGS_DECL> BaseType;
	public:
		void triggerCallbacks( VARIABLE_ARGS_IMPL )
		{
			if (BaseType::mTriggering)
			{
				//::logging::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while triggering again!!");
				return;
			}
			BaseType::mTriggering = true;
			if (multithreaded)
			{
				Lock lck( BaseType::mSC );
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				auto j = BaseType::mCallbacks.end();
				while( i != j )
				{
					if ( (*i->cb)(VARIABLE_ARGS_USE) )
					//if ( (**i)( VARIABLE_ARGS_USE ) )
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
			else
			{
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				auto j = BaseType::mCallbacks.end();
				while( i != j )
				{
					if ( (*i->cb)(VARIABLE_ARGS_USE) )
					//if ( (**i)( VARIABLE_ARGS_USE ) )
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
		}
	protected:
		CallbackSubscriptorNotTyped():BaseType(){}
		CallbackSubscriptorNotTyped( const CallbackSubscriptorNotTyped& o2 ):BaseType( o2 ){}

	};

	////specialization for void arguments
	template <bool multithreaded,bool postSubscription,class TRet >
	class CallbackSubscriptorNotTyped<multithreaded,postSubscription,TRet,void> : public CallbackSubscriptor_Base<multithreaded,TRet,void>
	{
		typedef CallbackSubscriptor_Base<multithreaded, TRet,void> BaseType;
	public:
		void triggerCallbacks(  )
		{
			if (BaseType::mTriggering)
			{
				//::logging::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while triggering again!!");
				return;
			}
			BaseType::mTriggering = true;
			if (multithreaded)
			{
				Lock lck( BaseType::mSC );
				for( typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin(),j = BaseType::mCallbacks.end(); i != j; ++i )
					(*i->cb)();
				BaseType::mTriggering = false;
				if (!BaseType::mPendingOperation.empty())
					BaseType::_applyPendingOperations();
			}
			else
			{
				for( typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin(),j = BaseType::mCallbacks.end(); i != j; ++i )
					(*i->cb)();
				BaseType::mTriggering = false;
				if (!BaseType::mPendingOperation.empty())
					BaseType::_applyPendingOperations();
			}
		}
	protected:
		CallbackSubscriptorNotTyped():BaseType(){}
		CallbackSubscriptorNotTyped( const CallbackSubscriptorNotTyped& o2 ):BaseType( o2 ){}

	};
	////specialization for void arguments and postsubscription = true
	template <bool multithreaded,class TRet >
	class CallbackSubscriptorNotTyped<multithreaded,true,TRet,void> : public CallbackSubscriptor_Base<multithreaded,TRet,void>
	{
		typedef CallbackSubscriptor_Base<multithreaded, TRet,void> BaseType;
	public:

		void triggerCallbacks(  )
		{
			if (BaseType::mTriggering)
			{
				//::logging::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while triggering again!!");
				return;
			}
			BaseType::mTriggering = true;
			if (multithreaded)
			{
				Lock lck( BaseType::mSC );
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				auto j = BaseType::mCallbacks.end();
				while( i != j )
				{
					//if ( (**i)(  ) )
					if ( (*i->cb)() )
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
			else
			{
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				auto j = BaseType::mCallbacks.end();
				while( i != j )
				{
					//if ( (**i)(  ) )
					if ( (*i->cb)() )
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

		}
	protected:
		CallbackSubscriptorNotTyped():BaseType(){}
		CallbackSubscriptorNotTyped( const CallbackSubscriptorNotTyped& o2 ):BaseType( o2 ){}

	};
	template <class TCallback, bool postSubscription,class TRet, VARIABLE_ARGS >
	class CallbackSubscriptor : public CallbackSubscriptorNotTyped <false,postSubscription, TRet, VARIABLE_ARGS_DECL>
	{
        typedef CallbackSubscriptorNotTyped <false,postSubscription, TRet, VARIABLE_ARGS_DECL> BaseType;
	public:
		CallbackSubscriptor():BaseType(){}
		CallbackSubscriptor( const CallbackSubscriptor& o2 ):BaseType(o2) {}

	};
	//specialization for void arguments
	template <class TCallback, bool postSubscription, class TRet >
	class CallbackSubscriptor<TCallback, postSubscription, TRet,void> : public CallbackSubscriptorNotTyped <false, postSubscription, TRet, void>
	{
		typedef CallbackSubscriptorNotTyped <false, postSubscription, TRet> BaseType;
	public:
		CallbackSubscriptor() :BaseType() {}
		CallbackSubscriptor(const CallbackSubscriptor& o2) :BaseType(o2) {}
		int subscribeCallback(std::function< TRet()>&& callback, SubscriptionEmplacement se=SE_BACK)
		{
			return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(std::move(callback), ::core::use_function), se);
		}
		int subscribeCallback(const std::function< TRet()>& callback, SubscriptionEmplacement se=SE_BACK)
		{
			return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(callback, ::core::use_function), se);
		}
		/*int subscribeCallback(std::function< TRet()>& callback, SubscriptionEmplacement se)
		{
			return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(callback, ::core::use_function), se);
		}*/
		template <class U>
		int subscribeCallback(U&& callback, SubscriptionEmplacement se=SE_BACK)
		{
			return BaseType::subscribeCallback(::std::forward<U>(callback), se);
		}
		//left unimplemented
		bool unsubscribeCallback(std::function< TRet()>&& callback)
		{
			return false; //can't unsubscribe function<>, hasn't operator ==
		}
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
	//MultithreadCallbackSusbscriptor. Concurrent accesses are protected
/*	template <class TCallback, class TRet, VARIABLE_ARGS_NODEFAULT >
	class MTCallbackSubscriptor<TCallback, true, TRet, VARIABLE_ARGS_DECL> : public CallbackSubscriptorNotTyped <true, true, TRet, VARIABLE_ARGS_DECL>
	{
		typedef CallbackSubscriptorNotTyped <true, true, TRet, VARIABLE_ARGS_DECL> BaseType;
	public:
		MTCallbackSubscriptor() {}
		MTCallbackSubscriptor(const MTCallbackSubscriptor& o2) :BaseType(o2) {}

	};*/
	template <class TCallback, bool postSubscription,class TRet, VARIABLE_ARGS >
	class MTCallbackSubscriptor : public CallbackSubscriptorNotTyped <true,postSubscription, TRet, VARIABLE_ARGS_DECL>
	{
        typedef CallbackSubscriptorNotTyped <true,postSubscription, TRet, VARIABLE_ARGS_DECL> BaseType;
	public:
		MTCallbackSubscriptor():BaseType(){}
		MTCallbackSubscriptor( const MTCallbackSubscriptor& o2 ):BaseType(o2) {}
		
	};
	//specialization for void arguments
	template <class TCallback, bool postSubscription, class TRet >
	class MTCallbackSubscriptor<TCallback, postSubscription, TRet, void> : public CallbackSubscriptorNotTyped <true, postSubscription, TRet, void>
	{
		typedef CallbackSubscriptorNotTyped <true, postSubscription, TRet> BaseType;
	public:
		MTCallbackSubscriptor():BaseType() {}
		MTCallbackSubscriptor(const MTCallbackSubscriptor& o2) :BaseType(o2) {}
		int subscribeCallback(std::function< TRet()>&& callback, SubscriptionEmplacement se = SE_BACK)
		{
			return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(std::move(callback), ::core::use_function), se);
		}
		int subscribeCallback(const std::function< TRet()>& callback, SubscriptionEmplacement se = SE_BACK)
		{
			return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(callback, ::core::use_function), se);
		}
		/*int subscribeCallback(std::function< TRet()>& callback, SubscriptionEmplacement se)
		{
			return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(callback, ::core::use_function), se);
		}*/
		template <class U>
		int subscribeCallback(U&& callback, SubscriptionEmplacement se = SE_BACK)
		{
			return BaseType::subscribeCallback(::std::forward<U>(callback), se);
		}
		//left unimplemented
		bool unsubscribeCallback(std::function< TRet()>&& callback)
		{
			return false; //can't unsubscribe function<>, hasn't operator ==
		}
		template <class U>
		bool unsubscribeCallback(U&& callback)
		{
			return BaseType::unsubscribeCallback(::std::forward<U>(callback));
		}
		bool unsubscribeCallback(int id)
		{
			return BaseType::unsubscribeCallback(id);
		}
	};
		
#else
	template <bool multithreaded, class TRet, VARIABLE_ARGS >
	class CallbackSubscriptor_Base<multithreaded, TRet,VARIABLE_ARGS_DECL,void>
	{
	public:
		typedef Callback<TRet, VARIABLE_ARGS_DECL> CallbackType;
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
			if (multithreaded)
			{
				Lock lck( mSC );
				mCallbacks.clear();
			}
			else
			{
				mCallbacks.clear();
			}
		}

		template <class U>
		int subscribeCallback(U&& callback, SubscriptionEmplacement se=SE_BACK)
		{
			return subscribeCreatedCallback(new CallbackType(::std::forward<U>(callback), ::core::use_functor), se);
		}

		/**
		* special case for subscription with Callback already created. Takes ownership
		*/

		int subscribeCreatedCallback( CallbackType* cb, SubscriptionEmplacement se=SE_BACK )
		{			
			int result;
			if (multithreaded)
			{
				Lock lck( mSC );
				result = ++mCurrId;
				if (mTriggering)
				{
					//::logging::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while subscribing!!");
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
			}
			else {
				result = ++mCurrId;
				if (mTriggering)
				{
					//(::
//					::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while subscribing!!");
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
			}
			return result;
		}

		template <class U>
		bool unsubscribeCallback( U&& callback )
		{
			bool result = false;
			if (multithreaded)
			{
				Lock lck( mSC );
				CallbackType* auxiliar = new CallbackType( std::forward<U>(callback), ::core::use_functor );
				result = unsubscribeCreatedCallback(std::shared_ptr< CallbackType>(auxiliar));
			}
			else
			{
				CallbackType* auxiliar = new CallbackType( std::forward<U>(callback), ::core::use_functor );
				result = unsubscribeCreatedCallback(std::shared_ptr< CallbackType>(auxiliar));
			}
			return result;	
		}
		bool unsubscribeCallback(int id)
		{
			bool result = false;
			//ESTÁN MAL LOS LOCKS
			if (multithreaded)
			{
				Lock lck(mSC);
				if (mTriggering)
				{
					//::logging::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while unsubscribing!!");					
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
			}
			else
			{
				if (mTriggering)
				{
					//::logging::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while subscribing!!");
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
			if (multithreaded)
			{
				Lock lck(mSC);
				if (mTriggering)
				{
					//::logging::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while unsubscribing!!");
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
			}
			else
			{
				if (mTriggering)
				{
					//::logging::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while unsubscribing!!");
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
			}
			return result;
		}


	protected:
		CallbackListType mCallbacks;
		CriticalSection mSC;
		CallbackSubscriptor_Base( const CallbackSubscriptor_Base& o2 ):mTriggering(false)
		{
			if (multithreaded)
			{
				Lock lck( mSC );
				typename CallbackListType::const_iterator i,j;
				for( i = o2.mCallbacks.begin(),j = o2.mCallbacks.end(); i!=j;++i) 
					mCallbacks.push_back( CallbackInfo( std::shared_ptr<CallbackType>((*i).cb->clone()),++mCurrId) );
			}
			else
			{
				typename CallbackListType::const_iterator i,j;
				for( i = o2.mCallbacks.begin(),j = o2.mCallbacks.end(); i!=j;++i) 
					mCallbacks.push_back( CallbackInfo(std::shared_ptr<CallbackType>((*i).cb->clone()),++mCurrId) );
			}
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

	template <bool multithreaded,bool postSubscription,class TRet, VARIABLE_ARGS >
	class CallbackSubscriptorNotTyped<multithreaded,postSubscription,TRet,VARIABLE_ARGS_DECL,void> : public CallbackSubscriptor_Base<multithreaded, TRet,VARIABLE_ARGS_DECL>
	{
		typedef CallbackSubscriptor_Base<multithreaded, TRet,VARIABLE_ARGS_DECL> BaseType;
	public:

		void triggerCallbacks( VARIABLE_ARGS_IMPL )
		{
			if (BaseType::mTriggering)
			{
				//::logging::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while triggering again!!");
				return;
			}
			BaseType::mTriggering = true;
			if (multithreaded)
			{
				Lock lck( BaseType::mSC );
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				auto j = BaseType::mCallbacks.end();
				while( i != j ) 
				{
					(*i->cb)(VARIABLE_ARGS_USE);
					++i;
				}
				BaseType::mTriggering = false;
				if (!BaseType::mPendingOperation.empty())
					BaseType::_applyPendingOperations();
			}
			else
			{
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				auto j = BaseType::mCallbacks.end();
				while( i != j ) 
				{
					(*i->cb)(VARIABLE_ARGS_USE);
					++i;
				}
				BaseType::mTriggering = false;
				if (!BaseType::mPendingOperation.empty())
					BaseType::_applyPendingOperations();
			}
		}
	protected:
		CallbackSubscriptorNotTyped():BaseType(){}
		CallbackSubscriptorNotTyped( const CallbackSubscriptorNotTyped& o2 ):BaseType( o2 ){}

	};
	//specialization for postSubscription == true
	template < bool multithreaded,class TRet, VARIABLE_ARGS >
	class CallbackSubscriptorNotTyped<multithreaded,true,TRet,VARIABLE_ARGS_DECL> : public CallbackSubscriptor_Base<multithreaded, TRet,VARIABLE_ARGS_DECL>
	{
		typedef CallbackSubscriptor_Base<multithreaded, TRet,VARIABLE_ARGS_DECL> BaseType;
	public:

		void triggerCallbacks( VARIABLE_ARGS_IMPL )
		{
			if (BaseType::mTriggering)
			{
				//::logging::Logger::getLogger()->debug("CallbackSubscriptor Callbacks are being triggered while triggering again!!");
				return;
			}
			BaseType::mTriggering = true;
			if (multithreaded)
			{
				Lock lck( BaseType::mSC );
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				while( i != BaseType::mCallbacks.end() )
				{
					//if ( (**i)( VARIABLE_ARGS_USE ) )
					if ( (*i->cb)(VARIABLE_ARGS_USE) )
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
			else
			{
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				while( i != BaseType::mCallbacks.end() )
				{
					//if ( (**i)( VARIABLE_ARGS_USE ) )
					if ( (*i->cb)(VARIABLE_ARGS_USE) )
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
			BaseType::mTriggering = false;
		}
	protected:
		CallbackSubscriptorNotTyped():BaseType(){}
		CallbackSubscriptorNotTyped( const CallbackSubscriptorNotTyped& o2 ):BaseType( o2 ){}
	};

	template <class TCallback, bool postSubscription,class TRet, VARIABLE_ARGS >
	class CallbackSubscriptor<TCallback, postSubscription,TRet,VARIABLE_ARGS_DECL,void> : public CallbackSubscriptorNotTyped<false,postSubscription,TRet,VARIABLE_ARGS_DECL,void>
	{
        typedef CallbackSubscriptorNotTyped<false,postSubscription,TRet,VARIABLE_ARGS_DECL,void> BaseType;
	public:
		CallbackSubscriptor():BaseType(){}
		CallbackSubscriptor( const CallbackSubscriptor& o2 ):BaseType( o2 ){}
		int subscribeCallback(std::function< TRet(VARIABLE_ARGS_DECL)>&& callback, SubscriptionEmplacement se=SE_BACK)
		{
			return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(std::move(callback), ::core::use_function), se);
		}
		int subscribeCallback(const std::function< TRet(VARIABLE_ARGS_DECL)>& callback, SubscriptionEmplacement se=SE_BACK)
		{
			return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(callback, ::core::use_function), se);
		}

		/*int subscribeCallback(std::function< TRet(VARIABLE_ARGS_DECL)>& callback, SubscriptionEmplacement se=SE_BACK)
		{
			return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(callback, ::core::use_function), se);
		}*/
		template <class U> int subscribeCallback(U&& functor, SubscriptionEmplacement se=SE_BACK)
		{
			return BaseType::subscribeCallback(std::forward<U>(functor), se );
		}
		bool unsubscribeCallback(std::function< TRet(VARIABLE_ARGS_DECL)>&& callback)
		{
			return false; //can't unsubscribe function
		}
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
	/*
	template <class TCallback, class TRet, VARIABLE_ARGS >
	class CallbackSubscriptor<TCallback,true,TRet,VARIABLE_ARGS_DECL> : public CallbackSubscriptorNotTyped<false,true,TRet,VARIABLE_ARGS_DECL>
	{
        typedef CallbackSubscriptorNotTyped<false,true,TRet,VARIABLE_ARGS_DECL> BaseType;
	public:
		CallbackSubscriptor():BaseType(){}
		CallbackSubscriptor( const CallbackSubscriptor& o2 ):BaseType( o2 ){}
	};*/
	/**
	* reentrant version
	*/
	template <class TCallback, bool postSubscription,class TRet, VARIABLE_ARGS >
	class MTCallbackSubscriptor<TCallback, postSubscription,TRet,VARIABLE_ARGS_DECL,void> : public CallbackSubscriptorNotTyped<true,postSubscription,TRet,VARIABLE_ARGS_DECL,void>
	{
        typedef CallbackSubscriptorNotTyped<true,postSubscription,TRet,VARIABLE_ARGS_DECL,void> BaseType;
	public:
		MTCallbackSubscriptor():BaseType(){}
		MTCallbackSubscriptor( const MTCallbackSubscriptor& o2 ):BaseType( o2 ){}
		int subscribeCallback(std::function< TRet(VARIABLE_ARGS_DECL)>&& callback, SubscriptionEmplacement se = SE_BACK)
		{
			return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(std::move(callback), ::core::use_function), se);
		}
		int subscribeCallback(const std::function< TRet(VARIABLE_ARGS_DECL)>& callback, SubscriptionEmplacement se = SE_BACK)
		{
			return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(callback, ::core::use_function), se);
		}

		/*int subscribeCallback(std::function< TRet(VARIABLE_ARGS_DECL)>& callback, SubscriptionEmplacement se=SE_BACK)
		{
			return BaseType::subscribeCreatedCallback(new typename BaseType::CallbackType(callback, ::core::use_function), se);
		}*/
		template <class U> int subscribeCallback(U&& functor, SubscriptionEmplacement se = SE_BACK)
		{
			return BaseType::subscribeCallback(std::forward<U>(functor), se);
		}
		bool unsubscribeCallback(std::function< TRet(VARIABLE_ARGS_DECL)>&& callback)
		{
			return false; //can't unsubscribe function
		}
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
	
	/*template <class TCallback, class TRet, VARIABLE_ARGS >
	class MTCallbackSubscriptor<TCallback,true,TRet,VARIABLE_ARGS_DECL> : public CallbackSubscriptorNotTyped<true,true,TRet,VARIABLE_ARGS_DECL>
	{
        typedef CallbackSubscriptorNotTyped<true,true,TRet,VARIABLE_ARGS_DECL> BaseType;
	public:
		MTCallbackSubscriptor():BaseType(){}
		MTCallbackSubscriptor( const MTCallbackSubscriptor& o2 ):BaseType( o2 ){}
	};*/
#endif
}