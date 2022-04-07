
///@cond HIDDEN_SYMBOLS
namespace mel
{
	namespace core
	{
	#if VARIABLE_NUM_ARGS == VARIABLE_MAX_ARGS
		template<class TRet, VARIABLE_ARGS>
		class  FunctionCallbackInterface_Base : public CallbackInterface<TRet, VARIABLE_ARGS_DECL>
		{
		protected:
			typedef std::function < TRet(VARIABLE_ARGS_DECL)> F;
			F mFunction;

		public:
			/**
			*
			* @param function function to be linked
			*/
			FunctionCallbackInterface_Base(F&& function) : mFunction(::std::move(function))
			{
			}
			FunctionCallbackInterface_Base(const F& function) : mFunction(function)
			{
			}
			FunctionCallbackInterface_Base( const FunctionCallbackInterface_Base& ev2 ) : mFunction( ev2.mFunction )
			{
			}
			//function<> objects haven�t operator=== available, so we have to invalidate here
			bool operator==(const CallbackInterface_Base<TRet, VARIABLE_ARGS_DECL>& ev2) const override
			{
				return false;
			}
		};
		/**
		* specialization for void arguments
		* @note really this shoudn't be necessary,  FunctionCallbackInterface_Base normal implementation should be ok, but in clang get an error I can't resolve now...
		*/
		template<class TRet>
		class  FunctionCallbackInterface_Base<TRet,void> : public CallbackInterface<TRet, void>
		{
		protected:
			typedef std::function < TRet()> F;
			F mFunction;

		public:
			/**
			*
			* @param function function to be linked
			*/
			FunctionCallbackInterface_Base(F&& function) : mFunction(::std::move(function))
			{
			}
			FunctionCallbackInterface_Base(const F& function) : mFunction(function)
			{
			}
			FunctionCallbackInterface_Base(const FunctionCallbackInterface_Base& ev2) : mFunction(ev2.mFunction)
			{
			}
			//function<> objects haven�t operator=== available, so we have to invalidate here
			bool operator==(const CallbackInterface_Base<TRet, void>& ev2) const override
			{
				return false;
			}
		};
		
		template<class TRet, VARIABLE_ARGS>
		class  FunctionCallbackInterface : public FunctionCallbackInterface_Base<TRet, VARIABLE_ARGS_DECL>
		{
		public:
			/**
			* @todo modificar al estilo de las cosas de mpl para permitir const F&
			*/
			FunctionCallbackInterface( typename FunctionCallbackInterface_Base<TRet, VARIABLE_ARGS_DECL>::F && function) : FunctionCallbackInterface_Base<TRet,VARIABLE_ARGS_DECL>(::std::move(function) ){}
			FunctionCallbackInterface(const typename FunctionCallbackInterface_Base<TRet, VARIABLE_ARGS_DECL>::F & function) : FunctionCallbackInterface_Base<TRet, VARIABLE_ARGS_DECL>(function) {}

			FunctionCallbackInterface( const FunctionCallbackInterface& ev2 ) : FunctionCallbackInterface_Base<TRet,VARIABLE_ARGS_DECL>( ev2 ){}
			TRet operator()(VARIABLE_ARGS_IMPL)
			{
				return FunctionCallbackInterface_Base<TRet,VARIABLE_ARGS_DECL>::mFunction( VARIABLE_ARGS_USE );
			}
			//function<> objects haven�t operator=== available, so we have to invalidate here
			bool operator==(const FunctionCallbackInterface& ev2) const 
			{
				return false;
			}
			CallbackInterface<TRet, VARIABLE_ARGS_DECL> * clone() const
			{
				return new FunctionCallbackInterface<TRet,  VARIABLE_ARGS_DECL>(*(FunctionCallbackInterface<TRet,  VARIABLE_ARGS_DECL>*)this);
			}
		};
		//specialization for void parameters
		template<class TRet>
		class  FunctionCallbackInterface<TRet,void> : public FunctionCallbackInterface_Base<TRet>
		{
		public:
			FunctionCallbackInterface(typename FunctionCallbackInterface_Base<TRet, void>::F && function) : FunctionCallbackInterface_Base<TRet>(std::move(function)) {}
			FunctionCallbackInterface(const typename FunctionCallbackInterface_Base<TRet, void>::F & function) : FunctionCallbackInterface_Base<TRet>(function) {}

			FunctionCallbackInterface( const FunctionCallbackInterface& ev2 ) : FunctionCallbackInterface_Base<TRet>( ev2 ){}
			TRet operator()()
			{
				return FunctionCallbackInterface_Base<TRet>::mFunction( );
			}
			bool operator==(const FunctionCallbackInterface<TRet, void>& ev2) const
			{
				return false;
			}
			CallbackInterface<TRet, void> * clone() const {
				return new FunctionCallbackInterface<TRet, void>(*(FunctionCallbackInterface<TRet>*)this);
			}
		};

	#else
		template<class TRet, VARIABLE_ARGS>
		class  FunctionCallbackInterface_Base<TRet,VARIABLE_ARGS_DECL,void> : public CallbackInterface<TRet, VARIABLE_ARGS_DECL>
		{
		protected:
			typedef std::function < TRet( VARIABLE_ARGS_DECL ) > F;
			F mFunction;

		public:
			/**
			*
			* @param function the function to be bound
			*/
			FunctionCallbackInterface_Base( F&& function):mFunction(std::move(function) )
			{
			}
			FunctionCallbackInterface_Base(const F& function) :mFunction(function)
			{
			}
			/*FunctionCallbackInterface_Base(F& function) :mFunction(function)
			{
			}*/
			FunctionCallbackInterface_Base( const FunctionCallbackInterface_Base& ev2 ) : mFunction( ev2.mFunction )
			{
			}
			bool operator==(const CallbackInterface_Base<TRet, VARIABLE_ARGS_DECL>& ev2) const override
			{
				return false;
			}
		};

		template<class TRet, VARIABLE_ARGS>
		class  FunctionCallbackInterface<TRet,VARIABLE_ARGS_DECL,void> : public FunctionCallbackInterface_Base<TRet, VARIABLE_ARGS_DECL>
		{
		public:
			FunctionCallbackInterface( typename FunctionCallbackInterface_Base<TRet,VARIABLE_ARGS_DECL>::F && function) : FunctionCallbackInterface_Base<TRet,  VARIABLE_ARGS_DECL>(::std::move(function) ){}
			FunctionCallbackInterface(const typename FunctionCallbackInterface_Base<TRet, VARIABLE_ARGS_DECL>::F & function) : FunctionCallbackInterface_Base<TRet, VARIABLE_ARGS_DECL>(function) {}
			FunctionCallbackInterface( const FunctionCallbackInterface& ev2 ) : FunctionCallbackInterface_Base<TRet,  VARIABLE_ARGS_DECL>( ev2 ){}
			TRet operator()(VARIABLE_ARGS_IMPL) override
			{
				return FunctionCallbackInterface_Base<TRet, VARIABLE_ARGS_DECL>::mFunction( VARIABLE_ARGS_USE );
			}
			//function<> objects haven�t operator=== available, so we have to invalidate here
			bool operator==(const FunctionCallbackInterface<TRet,VARIABLE_ARGS_DECL,void>& ev2) const 
			{
				return false;
			}
			CallbackInterface<TRet, VARIABLE_ARGS_DECL>* clone() const override
			{
				return new FunctionCallbackInterface<TRet, VARIABLE_ARGS_DECL>(*(FunctionCallbackInterface<TRet, VARIABLE_ARGS_DECL>*)this);
			}
		};


	#endif

	}
}

///@endcond
