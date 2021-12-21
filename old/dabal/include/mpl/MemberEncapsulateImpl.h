
/**
* @class MEncapsulate
* encapsulate an object and a member function in an functor
*
* Example:
* you have an object with a member function, say float Clase::funcion(int), and you want to create a functor for this
*
* Clase obj;
* MEncapsulate<Clase,float,int> me( &Clase::function,&obj );
*
* or use the herlper function:
*	makeMemberEncapsulate(&Class::execute,&obj )
*
* The you can use the new object as a normal functor: float result = me( 5 );
*/

namespace mpl
{
#if VARIABLE_NUM_ARGS == VARIABLE_MAX_ARGS
	///@cond HIDDEN_SYMBOLS
	struct const_function_t
	{
	};
	struct no_const_function_t
	{
	};
	const static const_function_t const_function = const_function_t();
	const static no_const_function_t no_const_function = no_const_function_t();

	template <bool isConst,class T, class PointerType, class TRet, VARIABLE_ARGS>
	class MEncapsulate_Base
	{

	protected:
		typedef TRet (T::*FNoConst)(VARIABLE_ARGS_DECL);
		typedef TRet (T::*FConst)(VARIABLE_ARGS_DECL) const;
		typedef typename _if<isConst,FConst,FNoConst>::Result F;

	public:
		MEncapsulate_Base():mOwner(0){};
		MEncapsulate_Base( const F& function, PointerType& obj ):mOwner(obj),mFunction(function)
		{
		}
		template <class F2>
        bool operator == ( const F2& ob2 ) const
        {
          return equality( Int2Type< Conversion<F2, MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL> >::exists >(),ob2 );
        }
		/*bool operator == ( const MEncapsulate_Base& ob2 ) const
		{
			return equality( Int2Type< true >(),ob2 );;
		}*/

    private:
        template <class F>
        bool equality( Int2Type<false>,const F&) const
        {
            return false;
        }
        bool equality( Int2Type<true >,const MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL>& me2) const
        {
            //return (mFunction == me2.mFunction && mOwner == me2.mOwner );
			//return  mOwner == me2.mOwner && ::mpl::Compare< ::mpl::ComparableTraits<F>::Result>::compare( mFunction,me2.mFunction);
			return  mOwner == me2.mOwner && ::mpl::equal<true>( mFunction,me2.mFunction);
        }

	protected:

		F mFunction;
		PointerType mOwner;
	};

	///@endcond
	template <bool isConst, class T, class PointerType, class TRet, VARIABLE_ARGS>
	class MEncapsulate : public MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL>
	{
		typedef typename MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL>::F F;
	public:
		MEncapsulate():MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL>(){};
		MEncapsulate( const F& function, PointerType& obj ):MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL>(function,obj)
		{
		}
		TRet operator()( VARIABLE_ARGS_IMPL )
		{
			return (MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL>::mOwner->*MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL>::mFunction)( VARIABLE_ARGS_USE);
		}
	};
	///@cond HIDDEN_SYMBOLS
	//specialization for void arguments
	template <bool isConst,class T, class PointerType, class TRet>
	class MEncapsulate_Base<isConst,T,PointerType,TRet>
	{

	protected:
		typedef TRet (T::*FNoConst)();
		typedef TRet (T::*FConst)() const;
		typedef typename _if<isConst,FConst,FNoConst>::Result F;

	public:
		MEncapsulate_Base():mOwner(0){};
		MEncapsulate_Base( const F& function, PointerType& obj ):mFunction(function), mOwner(obj)
		{
		}
		template <class F2>
        bool operator == ( const F2& ob2 ) const
        {
          return equality( Int2Type< Conversion<F2, MEncapsulate_Base<isConst,T,PointerType,TRet> >::exists >(),ob2 );
        }
		/*bool operator == ( const MEncapsulate_Base<isConst,T,PointerType,TRet>& ob2 ) const
		{
			return equality( Int2Type< true >(),ob2 );
		}*/
    private:
        template <class F>
        bool equality( Int2Type<false>,const F&) const
        {
            return false;
        }
        bool equality( Int2Type<true >,const MEncapsulate_Base<isConst,T,PointerType,TRet>& me2) const
        {
            //return (mFunction == me2.mFunction && mOwner == me2.mOwner );
			//return  mOwner == me2.mOwner && ::mpl::Compare< ::mpl::ComparableTraits<F>::Result>::compare( mFunction,me2.mFunction);
			return  mOwner == me2.mOwner && ::mpl::equal<true>( mFunction,me2.mFunction);
        }

	protected:

		F mFunction;
		//T* mOwner;
		PointerType mOwner;
	};

	///@endcond
	template <bool isConst,class T, class PointerType, class TRet >
	class MEncapsulate<isConst,T,PointerType,TRet> :  public MEncapsulate_Base<isConst,T,PointerType,TRet,void>
	{
		typedef typename MEncapsulate_Base<isConst,T,PointerType,TRet,void>::F F;

	public:
		MEncapsulate():MEncapsulate_Base<isConst,T,PointerType, TRet>(){};
		MEncapsulate( const F& function, PointerType& obj ):MEncapsulate_Base<isConst,T,PointerType,TRet>(function,obj)
		{
		}
		TRet operator()(  )
		{
			return (MEncapsulate_Base<isConst,T,PointerType,TRet,void>::mOwner->*MEncapsulate_Base<isConst,T,PointerType,TRet,void>::mFunction)( );
		}
	};


	template <class TRet, class T,VARIABLE_ARGS_NODEFAULT,class PointerType > inline
	MEncapsulate<false,T,PointerType,TRet,VARIABLE_ARGS_DECL> makeMemberEncapsulate( TRet (T::*F)(VARIABLE_ARGS_DECL),PointerType owner)
	{
		return MEncapsulate<false,T,PointerType,TRet,VARIABLE_ARGS_DECL>(F,owner);
	}

	template <class TRet, class T,VARIABLE_ARGS_NODEFAULT,class PointerType > inline
		MEncapsulate<false,T,PointerType,TRet,VARIABLE_ARGS_DECL> makeMemberEncapsulate( TRet (T::*F)(VARIABLE_ARGS_DECL),PointerType owner, no_const_function_t)
	{
		return MEncapsulate<false,T,PointerType,TRet,VARIABLE_ARGS_DECL>(F,owner);
	}
	/**
	* specialization for const members
	*/
	template <class TRet, class T,VARIABLE_ARGS_NODEFAULT,class PointerType > inline
		MEncapsulate<true,T,PointerType,TRet,VARIABLE_ARGS_DECL> makeMemberEncapsulate( TRet (T::*F)(VARIABLE_ARGS_DECL) const,PointerType owner,const_function_t)
	{
		return MEncapsulate<true,T,PointerType,TRet,VARIABLE_ARGS_DECL>(F,owner);
	}


	//specialization for void parameters
	template <class TRet, class T,class PointerType> inline
		MEncapsulate<false,T,PointerType,TRet> makeMemberEncapsulate( TRet (T::*F)(),PointerType owner)
	{
		return MEncapsulate<false,T,PointerType,TRet>(F,owner);
	}
	template <class TRet, class T,class PointerType> inline
		MEncapsulate<false,T,PointerType,TRet> makeMemberEncapsulate( TRet (T::*F)(),PointerType owner,no_const_function_t)
	{
		return MEncapsulate<false,T,PointerType,TRet>(F,owner);
	}

	template <class TRet, class T,class PointerType> inline
		MEncapsulate<true,T,PointerType,TRet> makeMemberEncapsulate( TRet (T::*F)() const,PointerType owner, const_function_t)
	{
		return MEncapsulate<true,T,PointerType,TRet>(F,owner);
	}


#else

	///@cond HIDDEN_SYMBOLS
	template <bool isConst, class T, class PointerType, class TRet, VARIABLE_ARGS>
	class MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL,void>
	{
	protected:
		typedef TRet (T::*FNoConst)(VARIABLE_ARGS_DECL);
		typedef TRet (T::*FConst)(VARIABLE_ARGS_DECL) const;
		typedef typename _if<isConst,FConst,FNoConst>::Result F;


	public:
		MEncapsulate_Base():mOwner(0){};
		MEncapsulate_Base( const F& function, PointerType& obj ):mFunction(function), mOwner(obj)
		{
		}
        template <class F2>
        bool operator == ( const F2& ob2 ) const
        {
           return equality( Int2Type< Conversion<F2, MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL> >::exists >(),ob2 );
        }
		/*bool operator == ( const MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL,void>& ob2 ) const
		{
			return equality( Int2Type< true >(),ob2 );
		}*/

    private:
        template <class F>
        bool equality( Int2Type<false>,const F&) const
        {
            return false;
        }
        bool equality( Int2Type<true >,const MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL>& me2) const
        {
            //return (mFunction == me2.mFunction && mOwner == me2.mOwner );
			//return  mOwner == me2.mOwner && ::mpl::Compare< ::mpl::ComparableTraits<F>::Result>::compare( mFunction,me2.mFunction);
			return  mOwner == me2.mOwner && ::mpl::equal<true>( mFunction,me2.mFunction);
        }
  protected:

		F mFunction;
		PointerType mOwner;
	};
	///@endcond
	template <bool isConst, class T, class PointerType, class TRet, VARIABLE_ARGS>
	class MEncapsulate<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL,void> : public MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL>
	{
		typedef typename MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL>::F F;
	public:
		MEncapsulate():MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL>(){};
		MEncapsulate( F function, PointerType& obj ):MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL>(function,obj){}
		TRet operator()( VARIABLE_ARGS_IMPL )
		{
			return (MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL>::mOwner->*MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL>::mFunction)( VARIABLE_ARGS_USE);
		}
		/*bool operator == ( const MEncapsulate<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL,void>& ob2 ) const
		{
			return MEncapsulate_Base<isConst,T,PointerType,TRet,VARIABLE_ARGS_DECL>::operator ==( ob2 );
		}*/

	};
	//helper function for argument deduction

	template <class TRet,class T, VARIABLE_ARGS,class PointerType> inline
	MEncapsulate<false,T,PointerType,TRet,VARIABLE_ARGS_DECL> makeMemberEncapsulate( TRet (T::*F)(VARIABLE_ARGS_DECL),PointerType owner)
	{
		return  MEncapsulate<false,T,PointerType,TRet,VARIABLE_ARGS_DECL>(F,owner);
	}
	/**
	* same as previous for coherence
	*/

	template <class TRet,class T, VARIABLE_ARGS,class PointerType > inline
		MEncapsulate<false,T,PointerType, TRet,VARIABLE_ARGS_DECL> makeMemberEncapsulate( TRet (T::*F)(VARIABLE_ARGS_DECL) ,PointerType owner,no_const_function_t)
	{
		return  MEncapsulate<false,T,PointerType,TRet,VARIABLE_ARGS_DECL>(F,owner);
	}

	template <class TRet,class T,VARIABLE_ARGS,class PointerType > inline
		MEncapsulate<true,T,PointerType,TRet,VARIABLE_ARGS_DECL> makeMemberEncapsulate( TRet (T::*F)(VARIABLE_ARGS_DECL) const,PointerType owner,const_function_t)
	{
		return  MEncapsulate<true,T,PointerType,TRet,VARIABLE_ARGS_DECL>(F,owner);
	}
	
#endif
}
