//DON'T INCLUDE DIRECTLY, USE Linker.h





namespace mpl
{
	/**
	* @class Linker1st
	* @brief Bind arguments to values for functors*
	* argument can be a functor (without parameter). use first template parameter, isFunctor, to set this mode
	* It's importan to note that, to desambiguate some situations, is necesary to put the number of parameters as a Int2Type
	* type in the third parameter of link1st function (except for only one parameter, it's not necessary)
	* Example:
	*
	*	you have the function:
	*		void* funcion( float, unsigned short, int )
	*
	*	link1st<false,void*,int,unsigned short>(funcion,5.6,Int2Type<3>()) -->gets a new function with signature void* f(unsigned short,int)
	* so you can do.
	*	link1st<false,void*,int,unsigned short>(funcion,5.6,Int2Type<3>())(14,10000) to execute the new function
	*
	* also, you can override all values doing:
	*	link1st<false,void*>(link1st<false,void*,int>(link1st<false,void*,int,unsigned short>(funcion,5.6,Int2Type<3>()),14,Int2Type<2>()),10000 )
	* so you just created a function with void arguments and all original thread with values 5.6,14,10000.
	*
	* if you want to use a functor as parameter:
	*   float floatParameter(){return 7.1f*time }; --> (time is an abstract value indicating it depends when it's executed)
	*   link1st<true,void*>( funcion,floatParameter,Int2Type<1>() ); --> then when you call operator() first parameter will be linked
	* with the result of execute floatParameter, just in the execution moment,not on declaring.

	* NOTE: as you can see, template arguments for function parameters are in inverse order
	* To adapt return values see ReturnAdaptor

	*/
#undef VARIABLE_ARGS_USE_SIN_ARG1
#undef VARIABLE_ARGS_IMPL_SIN_ARG1
	//chapucil gigantesco
#if VARIABLE_NUM_ARGS == 7
	#define VARIABLE_ARGS_USE_SIN_ARG1 arg2 coma arg3 coma arg4 coma arg5 coma arg6 coma arg7
	#define VARIABLE_ARGS_IMPL_SIN_ARG1 Arg2 arg2 coma Arg3 arg3 coma Arg4 arg4 coma Arg5 arg5 coma Arg6 arg6 coma Arg7 arg7
#elif VARIABLE_NUM_ARGS == 6
	#define VARIABLE_ARGS_USE_SIN_ARG1 arg2 coma arg3 coma arg4 coma arg5 coma arg6
	#define VARIABLE_ARGS_IMPL_SIN_ARG1 Arg2 arg2 coma Arg3 arg3 coma Arg4 arg4 coma Arg5 arg5 coma Arg6 arg6
#elif VARIABLE_NUM_ARGS == 5
	#define VARIABLE_ARGS_USE_SIN_ARG1 arg2 coma arg3 coma arg4 coma arg5
	#define VARIABLE_ARGS_IMPL_SIN_ARG1 Arg2 arg2 coma Arg3 arg3 coma Arg4 arg4 coma Arg5 arg5
#elif VARIABLE_NUM_ARGS == 4
	#define VARIABLE_ARGS_USE_SIN_ARG1 arg2 coma arg3 coma arg4
	#define VARIABLE_ARGS_IMPL_SIN_ARG1 Arg2 arg2 coma Arg3 arg3 coma Arg4 arg4
#elif VARIABLE_NUM_ARGS == 3
	#define VARIABLE_ARGS_USE_SIN_ARG1 arg2 coma arg3
	#define VARIABLE_ARGS_IMPL_SIN_ARG1 Arg2 arg2 coma Arg3 arg3
#elif VARIABLE_NUM_ARGS == 2
	#define VARIABLE_ARGS_USE_SIN_ARG1 arg2
	#define VARIABLE_ARGS_IMPL_SIN_ARG1 Arg2 arg2
#endif
#if VARIABLE_NUM_ARGS == VARIABLE_MAX_ARGS
	///@cond HIDDEN_SYMBOLS
	template <bool isFunctor,class T,class Arg1>
	class Linker1st_Base
	{

	public:
		Linker1st_Base( const T& functor, typename _if<isFunctor, typename TypeTraits<Arg1>::UnReferenced&,
			typename TypeTraits< typename TypeTraits<Arg1>::ParameterType>::NoConst>::Result arg1):
		  mFunctor(functor),
		  mArg( arg1 )
		{
		}
		template <class F2>
        bool operator == ( const F2& ob2 ) const
        {
           return equality( Int2Type< Conversion<F2, Linker1st_Base<isFunctor,T,Arg1> >::exists >(),ob2 );
        }

    private:
        template <class F>
        bool equality( Int2Type<false>,const F&) const
        {
            return false;
        }
        bool equality( Int2Type<true >,const Linker1st_Base<isFunctor,T,Arg1>& ob2) const
        {
            //return (mFunctor == ob2.mFunctor && mArg == ob2.mArg);
			//return Compare< ::mpl::ComparableTraits<T>::Result,T >::compare( mObject,ob2.mObject );
			//return ::mpl::Compare< ::mpl::ComparableTraits<Arg1>::Result>::compare( mArg,ob2.mArg) && ::mpl::Compare< ::mpl::ComparableTraits<T>::Result>::compare( mFunctor,ob2.mFunctor );
			return ::mpl::equal<true>( mArg, ob2.mArg ) && ::mpl::equal<true>( mFunctor, ob2.mFunctor ); 
        }

	protected:

		Arg1	mArg;
		T		mFunctor;
	};

	template < bool isFunctor,class T,class TRet, VARIABLE_ARGS > struct _call
	{
		template<class F> TRet operator()( F& functor,typename TypeTraits< typename TypeTraits<Arg1>::ParameterType>::NoConst arg,VARIABLE_ARGS_IMPL_SIN_ARG1 )
		{
			return functor( arg,VARIABLE_ARGS_USE_SIN_ARG1 );
		}
	};
	template < class T,class TRet, VARIABLE_ARGS_NODEFAULT > struct _call<true,T,TRet,VARIABLE_ARGS_DECL>
	{
		template<class F> TRet operator()( F& functor,Arg1& arg,VARIABLE_ARGS_IMPL_SIN_ARG1 )
		{
			return functor( arg(),VARIABLE_ARGS_USE_SIN_ARG1 );
		}
	};
	template <bool isFunctor,class T, class TRet, VARIABLE_ARGS>
	class Linker1st : public Linker1st_Base<isFunctor,T,Arg1>
	{

	public:
		Linker1st( const T& functor,typename _if<isFunctor,typename TypeTraits<Arg1>::UnReferenced&,
		typename TypeTraits< typename TypeTraits<Arg1>::ParameterType>::NoConst>::Result arg):
		Linker1st_Base<isFunctor,T,Arg1>(functor,arg){}
		TRet operator()( VARIABLE_ARGS_IMPL_SIN_ARG1 )
		{
			return mCall( Linker1st_Base<isFunctor,T,Arg1>::mFunctor,Linker1st_Base<isFunctor,T,Arg1>::mArg,VARIABLE_ARGS_USE_SIN_ARG1 );
		}
		/*bool operator == ( const Linker1st& ob2 ) const
		{
			return Linker1st_Base<isFunctor,T,Arg1>::operator ==( ob2 );
		}*/
	private:
		_call<isFunctor,T,TRet,VARIABLE_ARGS_DECL>	mCall;
	};

	///@endcond
	//helper function for argument deduction
	template <bool isFunctor,class TRet,VARIABLE_ARGS_INVERTED, class T > inline
	Linker1st<isFunctor,T,TRet,VARIABLE_ARGS_DECL> link1st( T functor,Arg1 arg,Int2Type<VARIABLE_NUM_ARGS>)
	{
		return Linker1st<isFunctor,T,TRet, VARIABLE_ARGS_DECL>( functor, arg );
	}
	//default for one argument
	template <bool isFunctor,class TRet,class Arg1, class T > inline
		Linker1st<isFunctor,T,TRet,Arg1> link1st( T functor,Arg1 arg)
	{
		return Linker1st<isFunctor,T,TRet, Arg1>( functor, arg );
	}
#else
	///@cond HIDDEN_SYMBOLS
	#if VARIABLE_NUM_ARGS == 1
	template < bool isFunctor,class T,class TRet, VARIABLE_ARGS>
	struct _call<isFunctor,T,TRet,VARIABLE_ARGS_DECL>
	{

		template<class F> TRet operator()( F& functor,typename TypeTraits< typename TypeTraits<Arg1>::ParameterType>::NoConst arg )
		{
			return functor( arg);
		}
	};

	template < class T,class TRet, VARIABLE_ARGS > struct _call<true,T,TRet,VARIABLE_ARGS_DECL>
	{
		//template<class F> TRet operator()( F& functor,Arg1 arg)
		template<class F> TRet operator()( F& functor,Arg1& arg)
		{
			return functor( arg() );
		}
	};
	#else
	template < bool isFunctor,class T,class TRet, VARIABLE_ARGS>
	struct _call<isFunctor,T,TRet,VARIABLE_ARGS_DECL>
	{
		//template<class F> TRet operator()( F& functor, typename TypeTraits< typename TypeTraits<const Arg1>::ParameterType>::NoConst arg,VARIABLE_ARGS_IMPL_SIN_ARG1 )
		//template<class F> TRet operator()( F& functor, Arg1& arg,VARIABLE_ARGS_IMPL_SIN_ARG1 )
		template<class F> TRet operator()( F& functor, typename TypeTraits< typename TypeTraits<Arg1>::ParameterType>::NoConst  arg,VARIABLE_ARGS_IMPL_SIN_ARG1 )
		{
			return functor( arg,VARIABLE_ARGS_USE_SIN_ARG1 );
		}
	};

	template < class T,class TRet, VARIABLE_ARGS > struct _call<true,T,TRet,VARIABLE_ARGS_DECL>
	{
		template<class F> TRet operator()( F& functor,Arg1& arg,VARIABLE_ARGS_IMPL_SIN_ARG1)
		{
			return functor( arg(),VARIABLE_ARGS_USE_SIN_ARG1 );
		}
	};
	#endif
	///@endcond
	template <bool isFunctor,class T, class TRet, VARIABLE_ARGS>
	class Linker1st<isFunctor,T,TRet,VARIABLE_ARGS_DECL,void> : public Linker1st_Base<isFunctor,T,Arg1>
	{

	public:
		//Linker1st( const T& functor,typename _if<isFunctor,typename TypeTraits<Arg1>::UnReferenced&,typename TypeTraits<Arg1>::ParameterType>::Result arg):
		Linker1st( const T& functor,
			typename _if<isFunctor,typename TypeTraits<Arg1>::UnReferenced&,
			typename TypeTraits< typename TypeTraits<Arg1>::ParameterType>::NoConst>::Result arg):
		Linker1st_Base<isFunctor,T,Arg1>(functor,arg){}

#if VARIABLE_NUM_ARGS == 1
		TRet operator()( )
		{
			return mCall( Linker1st_Base<isFunctor,T,Arg1>::mFunctor,Linker1st_Base<isFunctor,T,Arg1>::mArg );
		}


#else
		TRet operator()( VARIABLE_ARGS_IMPL_SIN_ARG1 )
		{
			return mCall( Linker1st_Base<isFunctor,T,Arg1>::mFunctor,Linker1st_Base<isFunctor,T,Arg1>::mArg,VARIABLE_ARGS_USE_SIN_ARG1 );
		}


#endif
		/*bool operator == ( const Linker1st<isFunctor,T,TRet,VARIABLE_ARGS_DECL,void>& ob2 ) const
		{
			return Linker1st_Base<isFunctor,T,Arg1>::operator ==( ob2 );
		}*/

	private:
		_call<isFunctor,T,TRet,VARIABLE_ARGS_DECL>	mCall;

	};

	//helper function for argument deduction
	template <bool isFunctor,class TRet,VARIABLE_ARGS_INVERTED, class T > inline
		Linker1st<isFunctor,T,TRet,VARIABLE_ARGS_DECL> link1st( T functor,Arg1 arg,Int2Type<VARIABLE_NUM_ARGS>)
	{
		return Linker1st<isFunctor,T,TRet, VARIABLE_ARGS_DECL>( functor, arg );
	}



#endif
}
