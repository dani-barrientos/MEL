/**
* @class ReturnAdaptor
* adapt functor to return specific values
*
* Example:
* you have the function:
*		int funcion( float,char )
* and you want to create a new function as this but returning a boolean, say true
*
*	returnAdaptor<float,char>( funcion, true )
* so this created a new function as "funcion" but returning true
*/


namespace mpl
{
#if VARIABLE_NUM_ARGS == VARIABLE_MAX_ARGS
	///@cond HIDDEN_SYMBOLS
	template <class TRet, class T >
	class ReturnAdaptor_Base
	{
	public:
		ReturnAdaptor_Base( const T& functor,typename TypeTraits<TRet>::ParameterType retorno ): mFunctor( functor ),mReturn( retorno )
		{
		}
		ReturnAdaptor_Base( const ReturnAdaptor_Base<TRet,T>& ob2 ): mFunctor( ob2.mFunctor ),
			mReturn( ob2.mReturn )
		{
		}
		template <class F>
		bool operator ==( const F& ob2 ) const
		{
		   // return mFunctor == ob2.mFunctor && mReturn == ob2.mReturn;
		   return equality( Int2Type< Conversion<F, ReturnAdaptor_Base<TRet,T> >::exists >(),ob2 );
		}
    private:
        template <class F>
        bool equality( Int2Type<false>,const F&) const
        {
            return false;
        }
        bool equality( Int2Type<true >,const ReturnAdaptor_Base<TRet,T>& ob2) const
        {
            //return (mFunctor == ob2.mFunctor && mReturn == ob2.mReturn);
			return ::mpl::equal<true>( mFunctor, ob2.mFunctor ) && ::mpl::equal<true>( mReturn, ob2.mReturn );
        }
	protected:
		T mFunctor;
		TRet mReturn;
	};
	//specialization for void return
	template <class T >
	class ReturnAdaptor_Base<void,T>
	{
	public:
		ReturnAdaptor_Base( const T& functor ): mFunctor( functor )
		{
		}
		ReturnAdaptor_Base( const ReturnAdaptor_Base<void,T>& ob2 ): mFunctor( ob2.mFunctor )
		{
		}
		template <class F>
		bool operator ==( const F& ob2 ) const
		{
            return equality( Int2Type< Conversion<F, ReturnAdaptor_Base<void,T> >::exists >(),ob2 );
		}
    private:
        template <class F>
        bool equality( Int2Type<false>,const F&) const
        {
            return false;
        }
        bool equality( Int2Type<true >,const ReturnAdaptor_Base<void,T>& ob2) const
        {
            //return (mFunctor == ob2.mFunctor);
			return ::mpl::equal<true>( mFunctor, ob2.mFunctor );
        }

	protected:
		T mFunctor;
	};

	///@endcond
	template <class TRet, class T, VARIABLE_ARGS >
	class ReturnAdaptor : public ReturnAdaptor_Base<TRet, T >
	{
	public:
		ReturnAdaptor( const T& functor, typename TypeTraits<TRet>::ParameterType retorno ):ReturnAdaptor_Base<TRet, T >( functor, retorno ){};
		ReturnAdaptor( const ReturnAdaptor& ob2 ): ReturnAdaptor_Base<TRet, T >( ob2 ){};

		TRet operator()( VARIABLE_ARGS_IMPL )
		{
			ReturnAdaptor_Base<TRet, T >::mFunctor( VARIABLE_ARGS_USE );
			return ReturnAdaptor_Base<TRet, T >::mReturn;
		}
		/*bool operator ==( const ReturnAdaptor& ob2 ) const
		{
			// return mFunctor == ob2.mFunctor && mReturn == ob2.mReturn;
			return ReturnAdaptor_Base<TRet, T >::operator ==( ob2 );
		}*/

	};
	///@cond HIDDEN_SYMBOLS
	//specialization for void return
	template <class T, VARIABLE_ARGS_NODEFAULT >
	class ReturnAdaptor<void,T,VARIABLE_ARGS_DECL> : public ReturnAdaptor_Base<void, T >
	{
	public:
		ReturnAdaptor( const T& functor):ReturnAdaptor_Base<void, T >( functor ){};
		ReturnAdaptor( const ReturnAdaptor& ob2 ): ReturnAdaptor_Base<void, T >( ob2 ){};

		void operator()( VARIABLE_ARGS_IMPL )
		{
			mFunctor( VARIABLE_ARGS_USE );
		}
		/*bool operator ==( const ReturnAdaptor<void,T,VARIABLE_ARGS_DECL>& ob2 ) const
		{
			// return mFunctor == ob2.mFunctor && mReturn == ob2.mReturn;
			return ReturnAdaptor_Base<void, T >::operator ==( ob2 );
		}*/

	};
	//specialization for void return and void arg
	template <class T >
	class ReturnAdaptor<void,T,void> : public ReturnAdaptor_Base<void, T >
	{
	public:
		ReturnAdaptor( const T& functor):ReturnAdaptor_Base<void, T >( functor ){};
		ReturnAdaptor( const ReturnAdaptor& ob2 ): ReturnAdaptor_Base<void, T >( ob2 ){};

		void operator()(  )
		{
			ReturnAdaptor_Base<void, T >::mFunctor(  );
		}
		/*bool operator ==( const ReturnAdaptor<void,T,void>& ob2 ) const
		{
			// return mFunctor == ob2.mFunctor && mReturn == ob2.mReturn;
			return ReturnAdaptor_Base<void, T >::operator ==( ob2 );
		}*/

	};

//specialization for void arguments
	template <class TRet,class T>
	class ReturnAdaptor<TRet,T,void> : public ReturnAdaptor_Base<TRet,T>
	{
	public:
		ReturnAdaptor( const T& functor, typename TypeTraits<TRet>::ParameterType retorno ):ReturnAdaptor_Base<TRet,T>( functor, retorno ){};
		ReturnAdaptor( const ReturnAdaptor& ob2 ): ReturnAdaptor_Base<TRet,T>( ob2 ){};
		TRet operator()()
		{
			ReturnAdaptor_Base<TRet,T>::mFunctor();
			return ReturnAdaptor_Base<TRet,T>::mReturn;
		}
		/*bool operator ==( const ReturnAdaptor<TRet,T,void>& ob2 ) const
		{
			// return mFunctor == ob2.mFunctor && mReturn == ob2.mReturn;
			return ReturnAdaptor_Base<TRet,T>::operator ==( ob2 );
		}*/
	};
	///@endcond
	template < VARIABLE_ARGS_NODEFAULT,class T,class TRet > inline
	ReturnAdaptor<TRet,T,VARIABLE_ARGS_DECL> returnAdaptor( T functor, TRet retorno )
	{
		return ReturnAdaptor<TRet,T,VARIABLE_ARGS_DECL>( functor, retorno );
	}

	/*template < VARIABLE_ARGS_NODEFAULT,class T> inline
		ReturnAdaptor<void,T,VARIABLE_ARGS_DECL> returnAdaptorVoid( T functor )
	{
		return ReturnAdaptor<void,T,VARIABLE_ARGS_DECL>( functor);
	}*/
	//TODO incompleto, tengo que hacerlo para parametros variables!!
	template < class T> inline
		ReturnAdaptor<void,T,void> returnAdaptorVoid( T functor )
	{
		return ReturnAdaptor<void,T,void>( functor);
	}

#else
	template <class TRet, class T, VARIABLE_ARGS >
	class ReturnAdaptor<TRet, T, VARIABLE_ARGS_DECL> : public ReturnAdaptor_Base<TRet, T >
	{
	public:
		ReturnAdaptor( T functor, typename TypeTraits<TRet>::ParameterType retorno ):ReturnAdaptor_Base<TRet, T >( functor, retorno ){};
		ReturnAdaptor( const ReturnAdaptor& ob2 ): ReturnAdaptor_Base<TRet, T >( ob2 ){};
		TRet operator()( VARIABLE_ARGS_IMPL )
		{
			ReturnAdaptor_Base<TRet, T >::mFunctor( VARIABLE_ARGS_USE );
			return ReturnAdaptor_Base<TRet, T >::mReturn;
		}
		/*bool operator ==( const ReturnAdaptor<TRet, T, VARIABLE_ARGS_DECL>& ob2 ) const
		{
			// return mFunctor == ob2.mFunctor && mReturn == ob2.mReturn;
			return ReturnAdaptor_Base<TRet, T >::operator ==( ob2 );
		}*/
	};
	/*//specialization for void return
	template <class T, VARIABLE_ARGS >
	class ReturnAdaptor<void, T, VARIABLE_ARGS_DECL> : public ReturnAdaptor_Base<void, T >
	{
	public:
		ReturnAdaptor( T functor ):ReturnAdaptor_Base( functor ){};
		ReturnAdaptor( const ReturnAdaptor& ob2 ): ReturnAdaptor_Base( ob2 ){};
		void operator()( VARIABLE_ARGS_IMPL )
		{
			mFunctor( VARIABLE_ARGS_USE );

		}

	};*/

	template < VARIABLE_ARGS,class T,class TRet > inline
	ReturnAdaptor<TRet,T,VARIABLE_ARGS_DECL> returnAdaptor( T functor, TRet retorno )
	{
		return ReturnAdaptor<TRet,T,VARIABLE_ARGS_DECL>( functor, retorno );
	}
	template < VARIABLE_ARGS,class T> inline
		ReturnAdaptor<void,T,VARIABLE_ARGS_DECL> returnAdaptorVoid( T functor )
	{
		return ReturnAdaptor<void,T,VARIABLE_ARGS_DECL>( functor );
	}

#endif
}
