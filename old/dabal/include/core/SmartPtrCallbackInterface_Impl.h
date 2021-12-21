/**
* Callback interface for objects with SmartPtr capability.
* It's used by Callback class when it's constructed from a IRefCount pointer (or with same interface for SmartPtr)
*/
namespace core
{
#if VARIABLE_NUM_ARGS == VARIABLE_MAX_ARGS
	template<class TRet, class F,VARIABLE_ARGS>
	class  SmartPtrCallbackInterface_Base : public CallbackInterface<TRet, VARIABLE_ARGS_DECL>
	{
		DABAL_CORE_OBJECT_TYPEINFO;
	protected:
		SmartPtr<F> mFunction;

	public:
		/**
		*
		* @param function the function to be bound
		*/
		SmartPtrCallbackInterface_Base(  F* function) : mFunction( function )
		{
		}

		SmartPtrCallbackInterface_Base( const SmartPtrCallbackInterface_Base& ev2 ) : mFunction( ev2.mFunction )
		{
		}
		bool operator==(const CallbackInterface_Base<TRet, VARIABLE_ARGS_DECL>& ev2) const
		{
			const SmartPtrCallbackInterface_Base <TRet,F,VARIABLE_ARGS_DECL> *ev;

			if (getMyType().instanceOf( ev2.getMyType() ) )
			{
				ev = static_cast<const SmartPtrCallbackInterface_Base <TRet,F,VARIABLE_ARGS_DECL> *>(&ev2);
				//return ( *ev->mFunction == *mFunction );
				return ::mpl::equal<true>(*ev->mFunction,*mFunction ); //by default, if object hasn't operator==, the return true

			}else
				return false;


		}
		CallbackInterface<TRet, VARIABLE_ARGS_DECL> * clone() const;

	};

	template <class TRet, class F,VARIABLE_ARGS_NODEFAULT >
	CallbackInterface<TRet, VARIABLE_ARGS_DECL> * SmartPtrCallbackInterface_Base<TRet,F,VARIABLE_ARGS_DECL>::clone() const
	{
		SmartPtrCallbackInterface_Base<TRet,F,VARIABLE_ARGS_DECL> *nuevo;
		nuevo = new SmartPtrCallbackInterface_Base<TRet,F,VARIABLE_ARGS_DECL>( *(SmartPtrCallbackInterface_Base<TRet,F,VARIABLE_ARGS_DECL>*)this );
		return nuevo;
	}

	template <class TRet,class F,VARIABLE_ARGS_NODEFAULT>
	DABAL_CORE_OBJECT_TYPEINFO_IMPL(SmartPtrCallbackInterface_Base <TRet coma F coma VARIABLE_ARGS_DECL >,CallbackInterface<TRet coma VARIABLE_ARGS_DECL>);

	template<class TRet, class F,VARIABLE_ARGS>
	class  SmartPtrCallbackInterface : public SmartPtrCallbackInterface_Base<TRet, F, VARIABLE_ARGS_DECL>
	{
			DABAL_CORE_OBJECT_TYPEINFO;
	public:
		/**
		* @todo modificar al estilo de las cosas de mpl para permitir const F&
		*/
		SmartPtrCallbackInterface( F* function) : SmartPtrCallbackInterface_Base<TRet,F,VARIABLE_ARGS_DECL>( function ){}

		SmartPtrCallbackInterface( const SmartPtrCallbackInterface& ev2 ) : SmartPtrCallbackInterface_Base<TRet,F,VARIABLE_ARGS_DECL>( ev2 ){}
		TRet operator()(VARIABLE_ARGS_IMPL)
		{
			return (*SmartPtrCallbackInterface_Base<TRet,F,VARIABLE_ARGS_DECL>::mFunction)( VARIABLE_ARGS_USE );
		}
	};

	//specialization for void parameters
	template<class TRet,class F>
	class  SmartPtrCallbackInterface<TRet, F,void> : public SmartPtrCallbackInterface_Base<TRet, F,void>
	{
	public:
		SmartPtrCallbackInterface(  F*function) : SmartPtrCallbackInterface_Base<TRet,F,void>( function ){}

		SmartPtrCallbackInterface( const SmartPtrCallbackInterface& ev2 ) : SmartPtrCallbackInterface_Base<TRet,F,void>( ev2 ){}
		TRet operator()()
		{
			return (*SmartPtrCallbackInterface_Base<TRet,F,void>::mFunction)( );
		}
	};


	template <class TRet,class F,VARIABLE_ARGS_NODEFAULT>
	DABAL_CORE_OBJECT_TYPEINFO_IMPL(SmartPtrCallbackInterface <TRet coma F coma VARIABLE_ARGS_DECL >,SmartPtrCallbackInterface_Base <TRet coma F coma VARIABLE_ARGS_DECL >);

#else
	template<class TRet, class F, VARIABLE_ARGS>
	class  SmartPtrCallbackInterface_Base<TRet,F,VARIABLE_ARGS_DECL,void> : public CallbackInterface<TRet, VARIABLE_ARGS_DECL>
	{
		DABAL_CORE_OBJECT_TYPEINFO;
	protected:
		SmartPtr<F> mFunction;

	public:
		/**
		*
		* @param function the function to be bound
		*/
		SmartPtrCallbackInterface_Base( F* function):mFunction( function )
		{
		}
		SmartPtrCallbackInterface_Base( const SmartPtrCallbackInterface_Base& ev2 ) : mFunction( ev2.mFunction )
		{
		}
		bool operator==(const CallbackInterface_Base<TRet, VARIABLE_ARGS_DECL>& ev2) const override
		{
			const SmartPtrCallbackInterface_Base <TRet,F,VARIABLE_ARGS_DECL> *ev;

			if (getMyType().instanceOf( ev2.getMyType() ) )
			{
				ev = static_cast<const SmartPtrCallbackInterface_Base <TRet,F,VARIABLE_ARGS_DECL> *>(&ev2);
				return ::mpl::equal<true>(*ev->mFunction,*mFunction ); //by default, if object hasn't operator==, the return true

			}else
				return false;


		}
		CallbackInterface<TRet, VARIABLE_ARGS_DECL> * clone() const override;


	};
	template <class TRet,class F,VARIABLE_ARGS>
	DABAL_CORE_OBJECT_TYPEINFO_IMPL(SmartPtrCallbackInterface_Base <TRet coma F coma VARIABLE_ARGS_DECL >,CallbackInterface<TRet coma VARIABLE_ARGS_DECL>);

	template<class TRet, class F,VARIABLE_ARGS>
	class  SmartPtrCallbackInterface<TRet,F,VARIABLE_ARGS_DECL,void> : public SmartPtrCallbackInterface_Base<TRet, F, VARIABLE_ARGS_DECL>
	{
		DABAL_CORE_OBJECT_TYPEINFO;
	public:
		SmartPtrCallbackInterface( F* function) : SmartPtrCallbackInterface_Base<TRet, F, VARIABLE_ARGS_DECL>( function ){}
		SmartPtrCallbackInterface( const SmartPtrCallbackInterface& ev2 ) : SmartPtrCallbackInterface_Base<TRet, F, VARIABLE_ARGS_DECL>( ev2 ){}
		TRet operator()(VARIABLE_ARGS_IMPL) override
		{
			return (*SmartPtrCallbackInterface_Base<TRet, F, VARIABLE_ARGS_DECL>::mFunction)( VARIABLE_ARGS_USE );
		}
	};

	template <class TRet, class F, VARIABLE_ARGS >
	CallbackInterface<TRet, VARIABLE_ARGS_DECL> * SmartPtrCallbackInterface_Base<TRet,F,VARIABLE_ARGS_DECL>::clone() const
	{
		SmartPtrCallbackInterface<TRet,F,VARIABLE_ARGS_DECL> *nuevo;
		nuevo = new SmartPtrCallbackInterface<TRet,F,VARIABLE_ARGS_DECL>( *(SmartPtrCallbackInterface<TRet,F,VARIABLE_ARGS_DECL>*)this );
		return nuevo;
	}

	template <class TRet,class F, VARIABLE_ARGS>
	DABAL_CORE_OBJECT_TYPEINFO_IMPL(SmartPtrCallbackInterface <TRet coma F coma VARIABLE_ARGS_DECL>,SmartPtrCallbackInterface_Base <TRet coma F coma VARIABLE_ARGS_DECL >);


#endif

}



