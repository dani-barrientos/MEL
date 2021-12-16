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
			CallbackType* cb;
			int id;  //callback id to use on unsubscriptionr when needed/neccesary
			CallbackInfo(CallbackType* acb, int aId) :cb(acb), id(aId) {}
		};
		int mCurrId;
	public:
		CallbackSubscriptor_Base() :mCurrId(0) {};
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
				for (auto c : mCallbacks)
					delete c.cb;
				mCallbacks.clear();
			}
			else
			{
				for (auto c : mCallbacks)
					delete c.cb;
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
				CallbackType auxiliar(::std::forward<U>(callback), ::core::use_functor );
				return unsubscribeCreatedCallback( auxiliar );
			}
			else
			{
				CallbackType auxiliar(::std::forward<U>(callback), ::core::use_functor );
				return unsubscribeCreatedCallback( auxiliar );
			}
		}
		bool unsubscribeCallback(int id)
		{
			bool result = false;
			//ESTÁN MAL LOS LOCKS
			if (multithreaded)
			{
				Lock lck(mSC);
				for (typename CallbackListType::iterator i = mCallbacks.begin(),j = mCallbacks.end();i != j; ++i)
				{
					if (i->id == id)
					{
						delete i->cb;
						mCallbacks.erase(i);
						result = true;
						break;
					}
				}			
				return result;
			}
			else
			{
				for (typename CallbackListType::iterator i = mCallbacks.begin(),j = mCallbacks.end();i != j; ++i)
				{
					if (i->id == id)
					{
						delete i->cb;
						mCallbacks.erase(i);
						result = true;
						break;
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
				if (se == SE_BACK) {
					mCallbacks.push_back(CallbackInfo(cb, (result = ++mCurrId)));
				}
				else {
					mCallbacks.push_front(CallbackInfo(cb, (result = ++mCurrId)));
				}
			}
			else {
				if (se == SE_BACK) {
					mCallbacks.push_back(CallbackInfo(cb, (result = ++mCurrId)));
				}
				else {
					mCallbacks.push_front(CallbackInfo(cb, (result = ++mCurrId)));
				}
			}
			return result;
		}
		/**
		* Unsubscribe an already existing callback
		* It looks for a callback equal to passed (cb), NOT THE POINTER
		* @param[in] cb the callback being desuscribed, which is not deleted
		* @return `true` if unsubscribed (`false` otherwise)
		*/
		bool unsubscribeCreatedCallback( const CallbackType& cb  )
		{
			bool result = false;
			if (multithreaded)
			{
				Lock lck( mSC );
				typename CallbackListType::iterator i = mCallbacks.begin();
				while( i != mCallbacks.end() )
				{
					if ( *i->cb == cb )
					{
						delete i->cb;
						mCallbacks.erase( i );
						i = mCallbacks.end();
						result = true;
					}else
					{
						++i;
					}
				}
			}
			else
			{
				typename CallbackListType::iterator i = mCallbacks.begin();
				while( i != mCallbacks.end() )
				{
					if ( *i->cb == cb )
					{
						delete i->cb;
						mCallbacks.erase( i );
						i = mCallbacks.end();
						result = true;
					}else
					{
						++i;
					}
				}
			}
			return result;
		}

	protected:
		CallbackListType mCallbacks;
		CriticalSection mSC;

		CallbackSubscriptor_Base( const CallbackSubscriptor_Base& o2 )
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
	};

	template <bool multithreaded,bool postSubscription,class TRet, VARIABLE_ARGS >
	class CallbackSubscriptorNotTyped : public CallbackSubscriptor_Base<multithreaded,TRet,VARIABLE_ARGS_DECL>
	{
		typedef CallbackSubscriptor_Base<multithreaded, TRet,VARIABLE_ARGS_DECL> BaseType;
	public:

		void triggerCallbacks( VARIABLE_ARGS_IMPL )
		{
			if (multithreaded)
			{
				Lock lck( BaseType::mSC );
				for( typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin(); i != BaseType::mCallbacks.end(); ++i )			
					(*i->cb)(VARIABLE_ARGS_USE);
			}
			else
			{
				for( typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin(); i != BaseType::mCallbacks.end(); ++i )			
					(*i->cb)(VARIABLE_ARGS_USE);
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
			if (multithreaded)
			{
				Lock lck( BaseType::mSC );
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				while( i != BaseType::mCallbacks.end() )
				{
					if ( (*i->cb)(VARIABLE_ARGS_USE) )
					//if ( (**i)( VARIABLE_ARGS_USE ) )
					{
						delete i->cb;
						i = BaseType::mCallbacks.erase( i );
					}else
					{
						++i;
					}
				}
			}
			else
			{
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				while( i != BaseType::mCallbacks.end() )
				{
					if ( (*i->cb)(VARIABLE_ARGS_USE) )
					//if ( (**i)( VARIABLE_ARGS_USE ) )
					{
						delete i->cb;
						i = BaseType::mCallbacks.erase( i );
					}else
					{
						++i;
					}
				}
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
			if (multithreaded)
			{
				Lock lck( BaseType::mSC );
				for( typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin(); i != BaseType::mCallbacks.end(); ++i )			
					(*i->cb)();
			}
			else
			{
				for( typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin(); i != BaseType::mCallbacks.end(); ++i )			
					(*i->cb)();
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
			if (multithreaded)
			{
				Lock lck( BaseType::mSC );
			typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				while( i != BaseType::mCallbacks.end() )
				{
					//if ( (**i)(  ) )
					if ( (*i->cb)() )
					{
						delete i->cb;
						i = BaseType::mCallbacks.erase( i );
					}else
					{
						++i;
					}
				}
			}
			else
			{
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				while( i != BaseType::mCallbacks.end() )
				{
					//if ( (**i)(  ) )
					if ( (*i->cb)() )
					{
						delete i->cb;
						i = BaseType::mCallbacks.erase( i );
					}else
					{
						++i;
					}
				}
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
			CallbackType* cb;
			int id;  //callback id to use on unsubscriptionr when needed/neccesary
			CallbackInfo(CallbackType* acb, int aId) :cb(acb), id(aId) {}
		};
		int mCurrId;

	public:
		//typedef list< CallbackType* > CallbackListType;
		typedef list< CallbackInfo > CallbackListType;
		CallbackSubscriptor_Base():mCurrId(0) {};
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
				for (auto c : mCallbacks)
					delete c.cb;
				mCallbacks.clear();
			}
			else
			{
				for (auto c : mCallbacks)
					delete c.cb;
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
				if (se == SE_BACK) {
					mCallbacks.push_back(CallbackInfo(cb, (result = ++mCurrId)));
				}
				else {
					mCallbacks.push_front(CallbackInfo(cb, (result = ++mCurrId)));
				}
			}
			else {
				if (se == SE_BACK) {
					mCallbacks.push_back(CallbackInfo(cb, (result = ++mCurrId)));
				}
				else {
					mCallbacks.push_front(CallbackInfo(cb, (result = ++mCurrId)));
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
				CallbackType auxiliar( std::forward<U>(callback), ::core::use_functor );
				result = unsubscribeCreatedCallback( auxiliar );
			}
			else
			{
				CallbackType auxiliar( std::forward<U>(callback), ::core::use_functor );
				result = unsubscribeCreatedCallback( auxiliar );
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
				for (typename CallbackListType::iterator i = mCallbacks.begin(), j = mCallbacks.end(); i != j; ++i)
				{
					if (i->id == id)
					{
						delete i->cb;
						mCallbacks.erase(i);
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
						delete i->cb;
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
		bool unsubscribeCreatedCallback( const CallbackType& cb  )
		{
			bool result = false;
			if (multithreaded)
			{
				Lock lck( mSC );
				typename CallbackListType::iterator i = mCallbacks.begin();
				while( i != mCallbacks.end() )
				{
					//if ( (**i) == cb )				
					if ( *i->cb == cb)
					{
						delete i->cb;
						mCallbacks.erase( i );
						i = mCallbacks.end();
						result = true;
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
				while( i != mCallbacks.end() )
				{
					//if ( (**i) == cb )				
					if ( *i->cb == cb)
					{
						delete i->cb;
						mCallbacks.erase( i );
						i = mCallbacks.end();
						result = true;
					}
					else
					{
						++i;
					}
				}
			}
			return result;
		}


	protected:
		CallbackListType mCallbacks;
		CriticalSection mSC;
		CallbackSubscriptor_Base( const CallbackSubscriptor_Base& o2 )
		{
			if (multithreaded)
			{
				Lock lck( mSC );
				typename CallbackListType::const_iterator i,j;
				for( i = o2.mCallbacks.begin(),j = o2.mCallbacks.end(); i!=j;++i) 
					mCallbacks.push_back( CallbackInfo((*i).cb->clone(),++mCurrId) );
			}
			else
			{
				typename CallbackListType::const_iterator i,j;
				for( i = o2.mCallbacks.begin(),j = o2.mCallbacks.end(); i!=j;++i) 
					mCallbacks.push_back( CallbackInfo((*i).cb->clone(),++mCurrId) );
			}
		}	
	};

	template <bool multithreaded,bool postSubscription,class TRet, VARIABLE_ARGS >
	class CallbackSubscriptorNotTyped<multithreaded,postSubscription,TRet,VARIABLE_ARGS_DECL,void> : public CallbackSubscriptor_Base<multithreaded, TRet,VARIABLE_ARGS_DECL>
	{
		typedef CallbackSubscriptor_Base<multithreaded, TRet,VARIABLE_ARGS_DECL> BaseType;
	public:

		void triggerCallbacks( VARIABLE_ARGS_IMPL )
		{
			if (multithreaded)
			{
				Lock lck( BaseType::mSC );
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				while( i != BaseType::mCallbacks.end() ) 
				{
					(*i->cb)(VARIABLE_ARGS_USE);
					++i;
				}
			}
			else
			{
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				while( i != BaseType::mCallbacks.end() ) 
				{
					(*i->cb)(VARIABLE_ARGS_USE);
					++i;
				}
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
			if (multithreaded)
			{
				Lock lck( BaseType::mSC );
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				while( i != BaseType::mCallbacks.end() )
				{
					//if ( (**i)( VARIABLE_ARGS_USE ) )
					if ( (*i->cb)(VARIABLE_ARGS_USE) )
					{
						delete i->cb;
						i = BaseType::mCallbacks.erase( i );
					}else
					{
						++i;
					}
				}
			}
			else
			{
				typename BaseType::CallbackListType::iterator i = BaseType::mCallbacks.begin();
				while( i != BaseType::mCallbacks.end() )
				{
					//if ( (**i)( VARIABLE_ARGS_USE ) )
					if ( (*i->cb)(VARIABLE_ARGS_USE) )
					{
						delete i->cb;
						i = BaseType::mCallbacks.erase( i );
					}else
					{
						++i;
					}
				}
			}
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