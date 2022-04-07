
namespace mel
{
	namespace mpl
	{
	#if VARIABLE_NUM_ARGS == VARIABLE_MAX_ARGS
		///@cond HIDDEN_SYMBOLS

		template <class T,class TRet,class NewArg>
		class ParamAdder_Base
		{
		public:
			ParamAdder_Base( T&& functor ):mFunctor( std::forward<T>(functor) ){};
			template <class F>
			bool operator ==( const F& ob2 ) const
			{
				return equality( Int2Type< Conversion<F, ParamAdder_Base<T,TRet,NewArg> >::exists >(),ob2 );
			}
		private:
			template <class F>
			bool equality( Int2Type<false>,const F&) const
			{
				return false;
			}
			bool equality( Int2Type<true >,const ParamAdder_Base<T,TRet,NewArg>& me2) const
			{
	
				return::mel::mpl::equal<true>( mFunctor,me2.mFunctor );
			}

		protected:
			T mFunctor;
		};
		///@endcond
		/**
		* @class ParamAdder
		* @brief create new functor with new param for operator() AFTER any other param
		* usage:
		*	suppose we have a function with signature: int f(  )
		*	and we need this function to adapt to a function with one argument of, for example, float type. Then do:
		*	addParam<int,float,void>( f );
		*	That create a function wich receives a float, so you call that function this way:
		*			addParam<int,float,void>( f )( 6.7f ); where first argument is return type, second is new type and else original arguments
		*
		* same is valid for functors with more params and other return values. Example:
		*		void f( int, float ) --> want to convert to void f( int, float, char );
		*	addParam<void,char,int,float>( f )(5,6.7f,'a');
		*
		*
		* this is neccesary in a lot of cases, for example, when chaining functors with different arguments types
		*/
		template <class T,class TRet,class NewArg,VARIABLE_ARGS>
		class ParamAdder 
		///@cond HIDDEN_SYMBOLS
		: public ParamAdder_Base<T,TRet,NewArg>
		///@endcond
		{
		public:
			ParamAdder( T&& functor ):ParamAdder_Base<T,TRet,NewArg>(std::forward<T>(functor)){};

			TRet operator()( VARIABLE_ARGS_IMPL,NewArg newarg )
			{
				return ParamAdder_Base<T,TRet,NewArg>::mFunctor( VARIABLE_ARGS_USE );
			};
		};
		///@cond HIDDEN_SYMBOLS
		//specialization for void arguments
		template <class T, class TRet, class NewArg >
		class ParamAdder<T,TRet,NewArg,void> : public ParamAdder_Base<T,TRet,NewArg>
		{
		public:
			ParamAdder( T&& functor ): ParamAdder_Base<T,TRet,NewArg>( std::forward<T>(functor) ){};
			TRet operator()( NewArg )
			{
				return ParamAdder_Base<T,TRet,NewArg>::mFunctor();
			}
			/*bool operator ==( const ParamAdder<T,TRet,NewArg,void>& ob2 ) const
			{
				return ParamAdder_Base<T,TRet,NewArg>::operator ==( ob2 );
			}*/

		};
		///@endcond
		template <class TRet, class NewArg,VARIABLE_ARGS_NODEFAULT,class T> inline
			ParamAdder<T,TRet,VARIABLE_ARGS_DECL, NewArg> addParam( T functor )
		{
			return ParamAdder<T,TRet,NewArg, VARIABLE_ARGS_DECL>( std::forward<T>(functor) );
		}
	#else
		template <class T,class TRet,class NewArg,VARIABLE_ARGS>
		class ParamAdder<T,TRet,NewArg,VARIABLE_ARGS_DECL,void> : public ParamAdder_Base<T,TRet,NewArg>
		{
		public:
			ParamAdder( T&& functor ):ParamAdder_Base<T,TRet,NewArg>( std::forward<T>(functor) ){};

			TRet operator()( VARIABLE_ARGS_IMPL,NewArg )
			{
				return ParamAdder_Base<T,TRet,NewArg>::mFunctor( VARIABLE_ARGS_USE );
			};
			/*bool operator ==( const ParamAdder<T,TRet,NewArg,VARIABLE_ARGS_DECL,void>& ob2 ) const
			{
				return ParamAdder_Base<T,TRet,NewArg>::operator ==( ob2 );
			}*/

		};


		template <class TRet, class NewArg,VARIABLE_ARGS,class T> inline
			ParamAdder<T,TRet,NewArg,VARIABLE_ARGS_DECL > addParam( T functor )
		{
			return ParamAdder<T,TRet,NewArg,VARIABLE_ARGS_DECL>( std::forward<T>(functor) );
		}
	#endif
		}

	/**
	* @class ParamAdder_prev
	* @brief create new functor with new param for operator() BEFORE any other param
	* usage:
	*	suppose we have a function with signature: int f(  )
	*	and we need this function to adapt to a function with one argument of, for example, float type. Then do:
	*	addParam<int,float,void>( f );
	*	That create a function wich receives a float, so you call that function this way:
	*			addParam_prev<int,float,void>( f )( 6.7f ); where first argument is return type, second is new type and else original arguments
	*
	* same is valid for functors with more params and other return values. Example:
	*		void f( int, float ) --> want to convert to void f( char,int, float);
	*	addParam_prev<void,char,int,float>( f )('a',5,6.7f);
	*
	*
	* this is neccesary in a lot of cases, for example, when chaining functors with different arguments types
	* NOTE not use a separate file or better organization for time reasons
	*/

	namespace mpl
	{
	#if VARIABLE_NUM_ARGS == VARIABLE_MAX_ARGS
		///@cond HIDDEN_SYMBOLS

		template <class T,class TRet,class NewArg>
		class ParamAdder_prev_Base
		{
		public:
			ParamAdder_prev_Base( T&& functor ):mFunctor( std::forward<T>(functor) ){};
			template <class F>
			bool operator ==( const F& ob2 ) const
			{
				return equality( Int2Type< Conversion<F, ParamAdder_prev_Base<T,TRet,NewArg> >::exists >(),ob2 );
			}
		private:
			template <class F>
			bool equality( Int2Type<false>,const F&) const
			{
				return false;
			}
			bool equality( Int2Type<true >,const ParamAdder_prev_Base<T,TRet,NewArg>& me2) const
			{
				return (mFunctor == me2.mFunctor);
			}

		protected:
			T mFunctor;
		};
		///@endcond
		template <class T,class TRet,class NewArg,VARIABLE_ARGS>
		class ParamAdder_prev : public ParamAdder_prev_Base<T,TRet,NewArg>
		{
		public:
			ParamAdder_prev( const T& functor ):ParamAdder_prev_Base<T,TRet,NewArg>(functor){};

			TRet operator()( NewArg newarg,VARIABLE_ARGS_IMPL )
			{
				return ParamAdder_prev_Base<T,TRet,NewArg>::mFunctor( VARIABLE_ARGS_USE );
			};
			/*bool operator ==( const ParamAdder_prev& ob2 ) const
			{
				return ParamAdder_prev_Base<T,TRet,NewArg>::operator ==( ob2 );
			}*/
		};
		///@cond HIDDEN_SYMBOLS
		//specialization for void arguments
		template <class T, class TRet, class NewArg >
		class ParamAdder_prev<T,TRet,NewArg,void> : public ParamAdder_prev_Base<T,TRet,NewArg>
		{
		public:
			ParamAdder_prev( const T& functor ): ParamAdder_prev_Base<T,TRet,NewArg>( functor ){};
			TRet operator()( NewArg )
			{
				return ParamAdder_prev_Base<T,TRet,NewArg>::mFunctor();
			}
			/*bool operator ==( const ParamAdder_prev<T,TRet,NewArg,void>& ob2 ) const
			{
				return ParamAdder_prev_Base<T,TRet,NewArg>::operator ==( ob2 );
			}*/

		};
		///@endcond
		template <class TRet, class NewArg,VARIABLE_ARGS_NODEFAULT,class T> inline
			ParamAdder_prev<T,TRet,VARIABLE_ARGS_DECL, NewArg> addParam_prev( T functor )
		{
			return ParamAdder_prev<T,TRet,NewArg, VARIABLE_ARGS_DECL>( functor );
		}
	#else
		template <class T,class TRet,class NewArg,VARIABLE_ARGS>
		class ParamAdder_prev<T,TRet,NewArg,VARIABLE_ARGS_DECL,void> : public ParamAdder_prev_Base<T,TRet,NewArg>
		{
		public:
			ParamAdder_prev( const T& functor ):ParamAdder_prev_Base<T,TRet,NewArg>( functor ){};

			TRet operator()( NewArg, VARIABLE_ARGS_IMPL)
			{
				return ParamAdder_prev_Base<T,TRet,NewArg>::mFunctor( VARIABLE_ARGS_USE );
			};
			/*bool operator ==( const ParamAdder_prev<T,TRet,NewArg,VARIABLE_ARGS_DECL,void>& ob2 ) const
			{
				return ParamAdder_prev_Base<T,TRet,NewArg>::operator ==( ob2 );
			}*/
		};


		template <class TRet, class NewArg,VARIABLE_ARGS,class T> inline
			ParamAdder_prev<T,TRet,NewArg,VARIABLE_ARGS_DECL > addParam_prev( T functor )
		{
			return ParamAdder_prev<T,TRet,NewArg,VARIABLE_ARGS_DECL>( functor );
		}
	#endif
		}
}