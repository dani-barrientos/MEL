
namespace core
{
#if VARIABLE_NUM_ARGS == VARIABLE_MAX_ARGS
	template<class TRet, class F,VARIABLE_ARGS>
	class  FunctorCallbackInterface_Base : public CallbackInterface<TRet, VARIABLE_ARGS_DECL>
	{
		DABAL_CORE_OBJECT_TYPEINFO;
	protected:
		typename ::std::decay<F>::type mFunction;
		//typename F mFunction;

	public:
		/**
		*
		* @param function the function to be bound
		*/		
		FunctorCallbackInterface_Base( F& function) : mFunction( function ){}
		FunctorCallbackInterface_Base(F&& function) :mFunction(::std::forward<F>(function)) {}
		
		FunctorCallbackInterface_Base( const FunctorCallbackInterface_Base& ev2 ) : mFunction( ev2.mFunction )
		{
		}
		bool operator==(const CallbackInterface_Base<TRet, VARIABLE_ARGS_DECL>& ev2) const
		{
			const FunctorCallbackInterface_Base <TRet,F,VARIABLE_ARGS_DECL> *ev;
			if ( getMyType().instanceOf( ev2.getMyType() ) )
			//if ( typeid(*this)==typeid(ev2))
			{
				ev = static_cast<const FunctorCallbackInterface_Base <TRet,F,VARIABLE_ARGS_DECL> *>(&ev2);
				//return (mFunction == ev->mFunction);
				return ::mpl::equal<true>( mFunction,ev->mFunction );


			}else
				return false;
		}
	};
	template <class TRet,class F,VARIABLE_ARGS_NODEFAULT>
	DABAL_CORE_OBJECT_TYPEINFO_IMPL(FunctorCallbackInterface_Base <TRet coma F coma VARIABLE_ARGS_DECL >,CallbackInterface<TRet coma VARIABLE_ARGS_DECL>);

	template<class TRet, class F,VARIABLE_ARGS>
	class  FunctorCallbackInterface : public FunctorCallbackInterface_Base<TRet, F, VARIABLE_ARGS_DECL>
	{
			DABAL_CORE_OBJECT_TYPEINFO;
	public:
		/**
		* @todo modificar al estilo de las cosas de mpl para permitir const F&
		*/
		FunctorCallbackInterface(F& function) : FunctorCallbackInterface_Base<TRet, F, VARIABLE_ARGS_DECL>(function) {}
		FunctorCallbackInterface( F&& function) : FunctorCallbackInterface_Base<TRet,F,VARIABLE_ARGS_DECL>(::std::forward<F>(function)){}

		FunctorCallbackInterface( const FunctorCallbackInterface& ev2 ) : FunctorCallbackInterface_Base<TRet,F,VARIABLE_ARGS_DECL>( ev2 ){}
		TRet operator()(VARIABLE_ARGS_IMPL)
		{
			return FunctorCallbackInterface_Base<TRet,F,VARIABLE_ARGS_DECL>::mFunction( VARIABLE_ARGS_USE );
		}
		bool operator==(const FunctorCallbackInterface& ev2) const
		{
			return FunctorCallbackInterface_Base<TRet, F, VARIABLE_ARGS_DECL>::operator ==( ev2 );
		}
		CallbackInterface<TRet, VARIABLE_ARGS_DECL> * clone() const;
	};
	template <class TRet, class F, VARIABLE_ARGS_NODEFAULT >
	CallbackInterface<TRet, VARIABLE_ARGS_DECL> * FunctorCallbackInterface<TRet, F, VARIABLE_ARGS_DECL>::clone() const
	{
		FunctorCallbackInterface<TRet, F, VARIABLE_ARGS_DECL> *nuevo;
		nuevo = new FunctorCallbackInterface<TRet, F, VARIABLE_ARGS_DECL>(*(FunctorCallbackInterface<TRet, F, VARIABLE_ARGS_DECL>*)this);
		return nuevo;
	}
	//specialization for void parameters
	template<class TRet,class F>
	class  FunctorCallbackInterface<TRet, F,void> : public FunctorCallbackInterface_Base<TRet, F,void>
	{
	public:
		FunctorCallbackInterface( F& function) : FunctorCallbackInterface_Base<TRet,F,void>( function ){}
		FunctorCallbackInterface(F&& function) : FunctorCallbackInterface_Base<TRet, F, void>(::std::forward<F>(function)) {}

		FunctorCallbackInterface( const FunctorCallbackInterface& ev2 ) : FunctorCallbackInterface_Base<TRet,F,void>( ev2 ){}
		TRet operator()()
		{
			return FunctorCallbackInterface_Base<TRet,F,void>::mFunction( );
		}
		bool operator==(const FunctorCallbackInterface<TRet, F,void>& ev2) const
		{
			return FunctorCallbackInterface_Base<TRet, F,void>::operator ==( ev2 );
		}
		CallbackInterface<TRet, void> * clone() const {
			return new FunctorCallbackInterface<TRet, F, void>(*(FunctorCallbackInterface<TRet, F, void>*)this);
		}
	};


	template <class TRet,class F,VARIABLE_ARGS_NODEFAULT>
	DABAL_CORE_OBJECT_TYPEINFO_IMPL(FunctorCallbackInterface <TRet coma F coma VARIABLE_ARGS_DECL >,FunctorCallbackInterface_Base <TRet coma F coma VARIABLE_ARGS_DECL >);

#else
	template<class TRet, class F, VARIABLE_ARGS>
	class  FunctorCallbackInterface_Base<TRet,F,VARIABLE_ARGS_DECL,void> : public CallbackInterface<TRet, VARIABLE_ARGS_DECL>
	{
		DABAL_CORE_OBJECT_TYPEINFO;
	protected:
		typename ::std::decay<F>::type mFunction;
		//F mFunction;

	public:
		/**
		*
		* @param function the function to be bound
		*/
		FunctorCallbackInterface_Base( F& function):mFunction( function ){}
		FunctorCallbackInterface_Base(F&& function) :mFunction(::std::forward<F>(function)){}

		FunctorCallbackInterface_Base( const FunctorCallbackInterface_Base& ev2 ) : mFunction( ev2.mFunction )
		{
		}
		bool operator==(const CallbackInterface_Base<TRet, VARIABLE_ARGS_DECL>& ev2) const override
		{
			const FunctorCallbackInterface_Base <TRet,F,VARIABLE_ARGS_DECL> *ev;
			/*
#ifdef _WINDOWS
			OutputDebugString(typeid(*this).name());
			OutputDebugString("\n");
			OutputDebugString(typeid(ev2).name());
			OutputDebugString("\n");
			OutputDebugString(typeid(mFunction).name());
			OutputDebugString("\n");
#endif*/
			if (getMyType().instanceOf(ev2.getMyType()))
			//if (typeid(*this) == typeid(ev2))
			{
				ev = static_cast<const FunctorCallbackInterface_Base <TRet,F,VARIABLE_ARGS_DECL> *>(&ev2);

				//return (mFunction == ev->mFunction);
				return ::mpl::equal<true>( mFunction,ev->mFunction );


			}else
				return false;
		}
	};
	template <class TRet,class F,VARIABLE_ARGS>
	DABAL_CORE_OBJECT_TYPEINFO_IMPL(FunctorCallbackInterface_Base <TRet coma F coma VARIABLE_ARGS_DECL >,CallbackInterface<TRet coma VARIABLE_ARGS_DECL>);

	template<class TRet, class F,VARIABLE_ARGS>
	class  FunctorCallbackInterface<TRet,F,VARIABLE_ARGS_DECL,void> : public FunctorCallbackInterface_Base<TRet, F, VARIABLE_ARGS_DECL>
	{
		DABAL_CORE_OBJECT_TYPEINFO;
	public:
		FunctorCallbackInterface( F& function) : FunctorCallbackInterface_Base<TRet, F, VARIABLE_ARGS_DECL>( function ){}
		FunctorCallbackInterface( F&& function) : FunctorCallbackInterface_Base<TRet, F, VARIABLE_ARGS_DECL>( ::std::forward<F>(function)) {}

		FunctorCallbackInterface( const FunctorCallbackInterface& ev2 ) : FunctorCallbackInterface_Base<TRet, F, VARIABLE_ARGS_DECL>( ev2 ){}
		TRet operator()(VARIABLE_ARGS_IMPL) override
		{
			return FunctorCallbackInterface_Base<TRet, F, VARIABLE_ARGS_DECL>::mFunction( VARIABLE_ARGS_USE );
		}
		bool operator==(const FunctorCallbackInterface<TRet,F,VARIABLE_ARGS_DECL,void>& ev2) const
		{
			return FunctorCallbackInterface_Base<TRet, F, VARIABLE_ARGS_DECL>::operator ==( ev2 );
		}
		CallbackInterface<TRet, VARIABLE_ARGS_DECL>* clone() const override
		{
			return new FunctorCallbackInterface<TRet, F, VARIABLE_ARGS_DECL>(*(FunctorCallbackInterface<TRet, F, VARIABLE_ARGS_DECL>*)this);
		}	
	};
	template <class TRet,class F, VARIABLE_ARGS>
	DABAL_CORE_OBJECT_TYPEINFO_IMPL(FunctorCallbackInterface <TRet coma F coma VARIABLE_ARGS_DECL>,FunctorCallbackInterface_Base <TRet coma F coma VARIABLE_ARGS_DECL >);


#endif

}



