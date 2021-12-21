using core::FunctorCallbackInterface;
//using core::SmartPtrCallbackInterface;
using core::FunctionCallbackInterface;
//using core::IRefCount;


namespace core
{
	class IRefCount;
	/**
	* @class Callback
	* @brief create Callback from functor
	* @todo no est� implementado el considerar cosas distintas a Functores, y posiblemente no sea necesario
	*/
#if VARIABLE_NUM_ARGS == VARIABLE_MAX_ARGS
		struct use_functor_t
		{
		};
		struct use_function_t
		{
		};
		const static use_functor_t use_functor = use_functor_t();
		const static use_function_t use_function = use_function_t();

		
		template <bool b,class T,class TRet,VARIABLE_ARGS >
		struct _CallbackCreator
		{
			
			static CallbackInterface<TRet, VARIABLE_ARGS_DECL>* create(T&& functor)
			{
				//typedef typename ::std::decay<T>::type NewType;
				//return new FunctorCallbackInterface<TRet, NewType, VARIABLE_ARGS_DECL>(::std::forward<NewType>(functor));
				return new FunctorCallbackInterface<TRet, typename ::std::remove_reference<T>::type, VARIABLE_ARGS_DECL>(::std::forward<T>(functor));
			}
		};
		template <class T, class TRet, VARIABLE_ARGS_NODEFAULT >
		struct _CallbackCreator<true, T, TRet, VARIABLE_ARGS_DECL>
		{
			static CallbackInterface<TRet, VARIABLE_ARGS_DECL>* create(T&& functor)
			{
				throw std::runtime_error("SmartPtrCallbackInterface not implemented");
				//return new SmartPtrCallbackInterface<TRet, typename mpl::TypeTraits<T>::PointeeType, VARIABLE_ARGS_DECL>(::std::forward<T>(functor));
			}
		};

		template< class TRet, VARIABLE_ARGS >
		class  Callback_Base
		{
		public:
			typedef TRet	ReturnType;

			/**
			* contructor from functor
			* @remarks second parameter only is used to distinguish from copy constructor.
			* Pass
			*/
			/*template<class T> Callback_Base( T& functor, const use_functor_t& )
			{
				//mCallBack = new FunctorCallbackInterface<TRet,T,VARIABLE_ARGS_DECL>( functor );
				mCallBack = _CallbackCreator< mpl::Conversion<T, IRefCount*>::exists, T, TRet, VARIABLE_ARGS_DECL>::create(functor);
			}*/
			template<class T> Callback_Base(T&& functor, const use_functor_t&)
			{
				mCallBack = _CallbackCreator< mpl::Conversion<T, IRefCount*>::exists, T, TRet, VARIABLE_ARGS_DECL>::create(::std::forward<T>(functor));
			}
			
			template<class T> Callback_Base(T&& functor, const use_function_t&)
			{
				mCallBack = new FunctionCallbackInterface<TRet, VARIABLE_ARGS_DECL>(::std::forward<T>(functor));
			}

			Callback_Base(const Callback_Base<TRet,VARIABLE_ARGS_DECL>& ev2)
			{
				mCallBack = 0;
				*this = ev2;
			}
			virtual ~Callback_Base()
			{
				delete mCallBack;
			};
			/**
			*
			* @param ev2 the callback being assigned from
			*/
			bool operator==(const Callback_Base<TRet, VARIABLE_ARGS_DECL>& ev2) const
			{
				return *mCallBack == *(ev2.mCallBack);
			}
			/**
			*
			* @param ev2 the callback being assigned from
			*/
			Callback_Base<TRet,VARIABLE_ARGS_DECL>& operator=(const Callback_Base<TRet,VARIABLE_ARGS_DECL>& ev2);

			virtual Callback_Base<TRet,VARIABLE_ARGS_DECL>* clone() const
			{
				return new Callback_Base<TRet,VARIABLE_ARGS_DECL>( *this );
			}
			/**
			* get callback for this event
			*/
			CallbackInterface<TRet,VARIABLE_ARGS_DECL> * getCallback()
			{
				return mCallBack;
			}

		protected:
			/**
			* function associated with this event
			*/
			CallbackInterface<TRet,VARIABLE_ARGS_DECL> *mCallBack;
		private:
			Callback_Base(){ mCallBack = 0;};


		};

		template<class TRet, VARIABLE_ARGS_NODEFAULT>
		Callback_Base<TRet,VARIABLE_ARGS_DECL>& Callback_Base<TRet,VARIABLE_ARGS_DECL>::operator=(const Callback_Base<TRet,VARIABLE_ARGS_DECL>& ev2)
		{
			if ( this != &ev2 )
			{
				delete mCallBack;
				mCallBack = (CallbackInterface<TRet,VARIABLE_ARGS_DECL>*)ev2.mCallBack->clone();
			}
			return *this;
		}

		//CLASS Callback
		template <class TRet, VARIABLE_ARGS>
		class Callback : public Callback_Base< TRet,VARIABLE_ARGS_DECL >
		{
		public:
			/**
			* execute event.
			* The number of agruments is variable and can't be propperly documented
			* following doxygen standards
			*/
			TRet operator()( VARIABLE_ARGS_IMPL )
			{
				return (*Callback_Base< TRet,VARIABLE_ARGS_DECL >::mCallBack)( VARIABLE_ARGS_USE );
			}
			/**
			* contructor from functor
			* @remarks second parameter only is used to distinguish from copy constructor
			*/
			template<class T> Callback( T&& functor,const use_functor_t& c) : Callback_Base<TRet,VARIABLE_ARGS_DECL>( ::std::forward<T>(functor), c){}
			template<class T> Callback( T&& functor, const use_function_t& c) : Callback_Base<TRet, VARIABLE_ARGS_DECL>(std::forward<T>(functor), c) {}
			Callback(const Callback<TRet,VARIABLE_ARGS_DECL>& ev2) : Callback_Base<TRet, VARIABLE_ARGS_DECL>( ev2 ){};
			Callback<TRet,VARIABLE_ARGS_DECL>* clone() const
			{
				return new Callback<TRet,VARIABLE_ARGS_DECL>( *this );
			}
		private:
			Callback(){ Callback_Base< TRet,VARIABLE_ARGS_DECL >::mCallBack = 0;};
		};

		////specialization for void arguments
		template<class TRet>
		class Callback<TRet,void> : public Callback_Base<TRet,void>
		{
		public:
			/**
			* execute event
			*/
			TRet operator()()
			{
				return (*Callback_Base<TRet,void>::mCallBack)(  );
			}

			/**
			* contructor from functor
			* @remarks second parameter only is used to distinguish from copy constructor
			*/
			//template<class T> Callback(T& functor, const use_functor_t& c) : Callback_Base<TRet, void>(functor, c) {}
			template<class T> Callback( T&& functor,const use_functor_t& c) : Callback_Base<TRet,void>( ::std::forward<T>(functor),c ){}
			template<class T> Callback(T&& functor, const use_function_t& c) : Callback_Base<TRet, void>(::std::forward<T>(functor), c) {}
			Callback(const Callback<TRet,void>& ev2) : Callback_Base<TRet, void>( ev2 ){};
			Callback<TRet,void>* clone() const
			{
				return new Callback<TRet,void>( *this );
			}
		private:
			Callback(){ Callback_Base<TRet,void>::mCallBack = 0;};
		};
	
		template <class TRet,class T>
			Callback<TRet,void>* makeEvent( T functor )
			{
				return new Callback<TRet,void>( functor, ::core::use_functor );
			}



#else

template <bool b,class T,class TRet,VARIABLE_ARGS >
struct _CallbackCreator<b,T,TRet,VARIABLE_ARGS_DECL,void>
{	
/*	static CallbackInterface<TRet, VARIABLE_ARGS_DECL>* create(T& functor)
	{
		//typedef typename ::std::decay<T>::type NewType;
		typedef typename T NewType;
		return new FunctorCallbackInterface<TRet, NewType, VARIABLE_ARGS_DECL>(::std::forward<T>(functor));
	}*/
	static CallbackInterface<TRet, VARIABLE_ARGS_DECL>* create(T&& functor)
	{

		//??TODO EL PROBLEMA ES QUE ESTE DECAY HACE QUE EL FORWARD VUELVA A EMITIR UN &&. EL PROBLEMA ES QUE LOS TIPOS DE LOS CALLBACKS NO CUADRARAN
		//typedef typename ::std::decay<T>::type NewType;
		//typedef typename ::std::remove_reference<T>::type NewType;
		//typedef T NewType;
//		return new FunctorCallbackInterface<TRet, NewType, VARIABLE_ARGS_DECL>(::std::forward<NewType>(functor));
		return new FunctorCallbackInterface<TRet, typename ::std::remove_reference<T>::type, VARIABLE_ARGS_DECL>(::std::forward<T>(functor));
	}
};
template <class T,class TRet,VARIABLE_ARGS >
struct _CallbackCreator<true,T,TRet,VARIABLE_ARGS_DECL,void>
{
	/*static CallbackInterface<TRet, VARIABLE_ARGS_DECL>* create(T& functor)
	{
		return new SmartPtrCallbackInterface<TRet, typename mpl::TypeTraits<T>::PointeeType, VARIABLE_ARGS_DECL>(functor);
	}*/
    static CallbackInterface<TRet,VARIABLE_ARGS_DECL>* create( T&& functor )
    {
		throw std::runtime_error("SmartPtrCallbackInterface not implemented");
        //return new SmartPtrCallbackInterface<TRet,typename mpl::TypeTraits<T>::PointeeType,VARIABLE_ARGS_DECL>(::std::forward<T>(functor) );
    }
};



template< class TRet, VARIABLE_ARGS >
	class  Callback_Base<TRet, VARIABLE_ARGS_DECL,void>
	{

	public:
		typedef TRet	ReturnType;
		//typedef Args    ArgType;

		Callback_Base(){ mCallBack = 0;};
		
	/*	template<class T> Callback_Base( T& functor,const use_functor_t& )
		{
			mCallBack = _CallbackCreator< mpl::Conversion<T,IRefCount*>::exists,T,TRet,VARIABLE_ARGS_DECL>::create( functor );
		}*/
		template<class T> Callback_Base(T&& functor, const use_functor_t&)
		{
			mCallBack = _CallbackCreator< mpl::Conversion<T, IRefCount*>::exists, T, TRet, VARIABLE_ARGS_DECL>::create(::std::forward<T>(functor));
			//mCallBack = new FunctorCallbackInterface<TRet, T, VARIABLE_ARGS_DECL>(::std::forward<T>(functor));
		}
		template<class T> Callback_Base(T&& functor, const use_function_t&)
		{
			mCallBack = new FunctionCallbackInterface<TRet, VARIABLE_ARGS_DECL>(::std::forward<T>(functor));			
		}
		Callback_Base(const Callback_Base<TRet,VARIABLE_ARGS_DECL>& ev2)
		{
			mCallBack = 0;
			*this = ev2;
		}
		virtual ~Callback_Base()
		{
			delete mCallBack;
		};
		/**
		*
		* @param ev2 the callback being assigned from
		*/
		bool operator==(const Callback_Base<TRet, VARIABLE_ARGS_DECL>& ev2) const
		{
			return *mCallBack == *(ev2.mCallBack);
		}
		/**
		*
		* @param ev2 the callback being assigned from
		*/
		Callback_Base<TRet,VARIABLE_ARGS_DECL> & operator=(const Callback_Base<TRet,VARIABLE_ARGS_DECL>& ev2);

		virtual Callback_Base<TRet,VARIABLE_ARGS_DECL>* clone() const
		{
			return new Callback_Base<TRet,VARIABLE_ARGS_DECL>( *this );
		}
		/**
		* get callback for this event
		*/
		CallbackInterface<TRet,VARIABLE_ARGS_DECL> * getCallback()
		{
			return mCallBack;
		}

	protected:
		/**
		* function associated with this event
		*/
		CallbackInterface<TRet,VARIABLE_ARGS_DECL> *mCallBack;
	};
	template<class TRet, VARIABLE_ARGS>
	Callback_Base<TRet,VARIABLE_ARGS_DECL> & Callback_Base<TRet,VARIABLE_ARGS_DECL>::operator=(const Callback_Base<TRet,VARIABLE_ARGS_DECL>& ev2)
	{
		if ( this != &ev2 )
		{
			delete mCallBack;
			mCallBack = (CallbackInterface<TRet,VARIABLE_ARGS_DECL>*)ev2.mCallBack->clone();
		}
		return *this;
	}

	template <class TRet, VARIABLE_ARGS>
	class Callback<TRet, VARIABLE_ARGS_DECL,void> : public Callback_Base< TRet,VARIABLE_ARGS_DECL >
	{
	public:
		/**
		* execute event
		* The number of agruments is variable and can't be propperly documented
		* following doxygen standards
		*/
		TRet operator()( VARIABLE_ARGS_IMPL )
		{
			return (*Callback_Base< TRet,VARIABLE_ARGS_DECL >::mCallBack)( VARIABLE_ARGS_USE );
		}
		Callback(){ Callback_Base< TRet,VARIABLE_ARGS_DECL >::mCallBack = 0;};

		/**
		* contructor from functor
		* @remarks second parameter only is used to distinguish from copy constructor
		*/
		template<class T> Callback( T&& functor,const use_functor_t& c) : Callback_Base<TRet,VARIABLE_ARGS_DECL>( ::std::forward<T>(functor),c){};
		template<class T> Callback( T&& functor, const use_function_t& c) : Callback_Base<TRet, VARIABLE_ARGS_DECL>(::std::forward<T>(functor), c) {};
		Callback(const Callback<TRet,VARIABLE_ARGS_DECL>& ev2) : Callback_Base<TRet, VARIABLE_ARGS_DECL>( ev2 ){};
		Callback<TRet,VARIABLE_ARGS_DECL>* clone() const
		{
			return new Callback<TRet,VARIABLE_ARGS_DECL>( *this );
		}

	};
#endif
}//end namespace


