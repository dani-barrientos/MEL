
namespace mel
{
	namespace mpl
	{
		
	#if VARIABLE_NUM_ARGS == VARIABLE_MAX_ARGS
		///@cond HIDDEN_SYMBOLS
		template < VARIABLE_ARGS>
		class NullFunctor_Base
		{
		public:
			NullFunctor_Base(){};
			template <class F2>
			bool operator == ( const F2& ob2 ) const
			{
				return Conversion<F2, NullFunctor_Base<VARIABLE_ARGS_DECL> >::exists ;
			}
		};
		///@endcond
		/**
	* @class NullFunctor
	* Always return void. If you need to return another type, use ReturnAdaptor over NullFunctor
	* create a functor with no operation. usefull if some code needs a functor but you don't want to execute
	* nothing
	*
	* Example:
	*/

		template < VARIABLE_ARGS>
		class NullFunctor 
		///@cond HIDDEN_SYMBOLS
		: public NullFunctor_Base< VARIABLE_ARGS_DECL>
		///@endcond
		{
		public:
			NullFunctor(){};
			void operator()( VARIABLE_ARGS_IMPL )
			{
			}
		};

		//specialization for void arguments
		template <>
		class NullFunctor<void> :public NullFunctor_Base<void>
		{
		public:
			NullFunctor(){};
			void operator()( )
			{
			}
		
		};

	#else
		///@cond HIDDEN_SYMBOLS
		template < VARIABLE_ARGS>
		class NullFunctor_Base<VARIABLE_ARGS_DECL,void>
		{
		public:
			NullFunctor_Base(){};
			template <class F2>
			bool operator == ( const F2& ob2 ) const
			{
				return Conversion<F2, NullFunctor_Base<VARIABLE_ARGS_DECL,void> >::exists;
			}
		};
		///@endcond
		template < VARIABLE_ARGS >
		class NullFunctor<VARIABLE_ARGS_DECL,void> : public NullFunctor_Base< VARIABLE_ARGS_DECL>
		{
		public:
			NullFunctor(){};
			void operator()( VARIABLE_ARGS_IMPL )
			{
			}
		};


	#endif
	}
}