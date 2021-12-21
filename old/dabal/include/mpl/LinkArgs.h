#pragma once
#include <mpl/Int2Type.h>
#include <mpl/typelist/TypeList.h>
#include <mpl/typelist/Length.h>
using mpl::EmptyType;
using mpl::nullType;
#include <mpl/TypeTraits.h>
using mpl::TypeTraits;
#include <mpl/Conversion.h>
using mpl::Conversion;
using mpl::typelist::TypeList;
#include <mpl/equal.h>
#include <mpl/typelist/Element.h>
//#include <mpl/isSame.h>
//#include <mpl/StaticAssert.h>
/*
void fTest(int&,char,XNode* )
{
}
XNode* createXNode()
{
return NULL;
}

int a;
linkFunctor< void,TYPELIST(XNode*,int&) >( fTest,mpl::_v2,'a',mpl::_v1 )( mpl::functorWrapper<XNode*>(createXNode),mpl::createRef(a));
*/

namespace mpl
{
	using mpl::typelist::Element;
	using mpl::typelist::Length;
	
    template <class F,class R> struct FunctorWrapper
    {
        FunctorWrapper( F& obj):mObj(obj){}

        inline operator R() const
        {
            return mObj();
        }
		bool operator == (const FunctorWrapper<F,R>& F2 ) const
		{
			return mObj == F2.mObj;
		}

        private:
            mutable F mObj;
    };
    //!helper function for argument deduction
    template <class R,class F> FunctorWrapper<F,R> functorWrapper( F f )
    {
        return FunctorWrapper<F,R>( f );
    }


    /*
        PENDIENTE:
            - comprobar que no haya mas fixed que variables
            - no tengo todavia claro que cosas permitir y no:llamar a la funcion con mas argumentos de los que admite,etc
    */
        struct VariablePosBase{};
        template <int n> struct VariablePos : VariablePosBase
        {
            enum {value = n};
        };

		const static VariablePos<1> _v1 = VariablePos<1>();
		const static VariablePos<2> _v2 = VariablePos<2>();
		const static VariablePos<3> _v3 = VariablePos<3>();
		const static VariablePos<4> _v4 = VariablePos<4>();
		const static VariablePos<5> _v5 = VariablePos<5>();
		const static VariablePos<6> _v6 = VariablePos<6>();

    template <class T>
    class Fixed1
    {
	public:
		inline bool operator == ( const Fixed1& ob2 ) const
		{
			return mpl::equal<true>(f1,ob2.f1); //TODO atencion que todavía no vale
		}
		T f1;
    protected:
        //typedef typename TypeTraits< typename TypeTraits<T>::ParameterType>::NoConst TArg1;
			typedef typename TypeTraits<T>::ParameterType TArg1;
			Fixed1( TArg1 arg ):f1( arg ){}
    };

    template <> class Fixed1<NullType>
    {
        public:
        template <class T> Fixed1( const T& ){}
    };

    template <class T>
    class Fixed2
    {
	public:
		inline bool operator == ( const Fixed2& ob2 ) const
		{
			return mpl::equal<true>(f2,ob2.f2);
		}
		T f2;
    protected:
        //typedef typename TypeTraits< typename TypeTraits<T>::ParameterType>::NoConst TArg2;
        typedef typename TypeTraits<T>::ParameterType TArg2;
        Fixed2( TArg2 arg ):f2( arg ){}


    };
    template <> class Fixed2<NullType>{
    public:
        template <class T> Fixed2( const T& ){}
    };
    template <class T>
    class Fixed3
    {
	public:
		inline bool operator == ( const Fixed3& ob2 ) const
		{
			return mpl::equal<true>(f3,ob2.f3); //TODO atencion que todavía no vale
		}
        T f3;
	protected:
        //typedef typename TypeTraits< typename TypeTraits<T>::ParameterType>::NoConst TArg3;
        typedef typename TypeTraits<T>::ParameterType TArg3;
        Fixed3( TArg3 arg ):f3( arg ){}


    };
    template <> class Fixed3<NullType>{
    public:
        template <class T> Fixed3( const T& ){}
    };

	template <class T>
	class Fixed4
	{
	public:
		inline bool operator == ( const Fixed4& ob2 ) const
		{
			return mpl::equal<true>(f4,ob2.f4); //TODO atencion que todavía no vale
		}
		T f4;
	protected:
		//typedef typename TypeTraits< typename TypeTraits<T>::ParameterType>::NoConst TArg4;
		typedef typename TypeTraits<T>::ParameterType TArg4;
		Fixed4( TArg4 arg ):f4( arg ){}



	};
	template <> class Fixed4<NullType>{
	public:
		template <class T> Fixed4( const T& ){}
	};

	template <class T>
	class Fixed5
	{
	public:
		inline bool operator == ( const Fixed5& ob2 ) const
		{
			return mpl::equal<true>(f5,ob2.f5); //TODO atencion que todavía no vale
		}
		T f5;
	protected:
		typedef typename TypeTraits<T>::ParameterType TArg5;
		Fixed5( TArg5 arg ):f5( arg ){}



	};
	template <> class Fixed5<NullType>{
	public:
		template <class T> Fixed5( const T& ){}
	};

	template <class T>
	class Fixed6
	{
	public:
		inline bool operator == ( const Fixed6& ob2 ) const
		{
			return mpl::equal<true>(f6,ob2.f6); //TODO atencion que todavía no vale
		}
	T f6;
	protected:
		typedef typename TypeTraits<T>::ParameterType TArg6;
		Fixed6( TArg6 arg ):f6( arg ){}
	


	};
	template <> class Fixed6<NullType>{
	public:
		template <class T> Fixed6( const T& ){}
	};

    template <class TListPos,int size> class Linker_Base;

    template <class TListPos> class Linker_Base< TListPos,1> :
        public Fixed1<
                typename _if< Conversion<typename Element< TListPos, 0,true>::Result,VariablePosBase>::exists,
                NullType,
                typename Element< TListPos, 0,true>::Result >::Result
                >
    {
        protected:

            typedef typename _if< Conversion<typename Element< TListPos, 0,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 0,true>::Result >::Result TArg1;
            typedef typename TypeTraits< typename Element< TListPos, 0,true>::Result >::ParameterType TArg1Ref;

            Linker_Base( TArg1Ref arg1 ):Fixed1<TArg1>( arg1 ){}

			bool operator == ( const Linker_Base<TListPos,1>& ob2 ) const
			{
				return mpl::equal<true>( *((Fixed1<TArg1>*)this),ob2 );
			}

    };
    template <class TListPos> class Linker_Base< TListPos,2> :
            public Fixed1<
                typename _if< Conversion<typename Element< TListPos, 0,true>::Result,VariablePosBase>::exists,
                NullType,
                typename Element< TListPos, 0,true>::Result >::Result
                >,
                public Fixed2<
                typename _if< Conversion<typename Element< TListPos, 1,true>::Result,VariablePosBase>::exists,
                NullType,
                typename Element< TListPos, 1,true>::Result >::Result
                >
    {
        protected:
            typedef typename _if< Conversion<typename Element< TListPos, 0,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 0,true>::Result >::Result TArg1;
            typedef typename _if< Conversion<typename Element< TListPos, 1,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 1,true>::Result >::Result TArg2;

            typedef typename TypeTraits<typename Element< TListPos, 0,true>::Result>::ParameterType TArg1Ref;
            typedef typename TypeTraits<typename Element< TListPos, 1,true>::Result>::ParameterType TArg2Ref;


            Linker_Base( TArg1Ref arg1,
                    TArg2Ref arg2 ):Fixed1<TArg1>( arg1 ),Fixed2<TArg2>( arg2 ){}

			bool operator == ( const Linker_Base<TListPos,2>& ob2 ) const
			{
				return mpl::equal<true>( *((Fixed1<TArg1>*)this),ob2 ) && 
					mpl::equal<true>( *((Fixed2<TArg2>*)this),ob2 );
			}

    };
    template <class TListPos> class Linker_Base< TListPos,3> :
            public Fixed1<
                typename _if< Conversion<typename Element< TListPos, 0,true>::Result,VariablePosBase>::exists,
                NullType,
                typename Element< TListPos, 0,true>::Result >::Result
                >,
            public Fixed2<
                typename _if< Conversion<typename Element< TListPos, 1,true>::Result,VariablePosBase>::exists,
                NullType,
                typename Element< TListPos, 1,true>::Result >::Result
                >,
            public Fixed3<
                typename _if< Conversion<typename Element< TListPos, 2,true>::Result,VariablePosBase>::exists,
                NullType,
                typename Element< TListPos, 2,true>::Result >::Result
                >

    {
        protected:
            typedef typename _if< Conversion<typename Element< TListPos, 0,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 0,true>::Result >::Result TArg1;
            typedef typename _if< Conversion<typename Element< TListPos, 1,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 1,true>::Result >::Result TArg2;
            typedef typename _if< Conversion<typename Element< TListPos, 2,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 2,true>::Result >::Result TArg3;

            typedef typename TypeTraits<typename Element< TListPos, 0,true>::Result>::ParameterType TArg1Ref;
            typedef typename TypeTraits<typename Element< TListPos, 1,true>::Result>::ParameterType TArg2Ref;
            typedef typename TypeTraits<typename Element< TListPos, 2,true>::Result>::ParameterType TArg3Ref;


        Linker_Base( TArg1Ref arg1,
                    TArg2Ref arg2,
                    TArg3Ref arg3 ):Fixed1<TArg1>( arg1 ),Fixed2<TArg2>( arg2 ),Fixed3<TArg3>( arg3 ){}

		bool operator == ( const Linker_Base<TListPos,3>& ob2 ) const
		{
			return mpl::equal<true>( *((Fixed1<TArg1>*)this),ob2 ) && 
				mpl::equal<true>( *((Fixed2<TArg2>*)this),ob2 ) &&
				mpl::equal<true>( *((Fixed3<TArg3>*)this),ob2 );
		}

		};

	template <class TListPos> class Linker_Base< TListPos,4> :
        public Fixed1<
            typename _if< Conversion<typename Element< TListPos, 0,true>::Result,VariablePosBase>::exists,
            NullType,
            typename Element< TListPos, 0,true>::Result >::Result
            >,
        public Fixed2<
            typename _if< Conversion<typename Element< TListPos, 1,true>::Result,VariablePosBase>::exists,
            NullType,
            typename Element< TListPos, 1,true>::Result >::Result
            >,
        public Fixed3<
            typename _if< Conversion<typename Element< TListPos, 2,true>::Result,VariablePosBase>::exists,
            NullType,
            typename Element< TListPos, 2,true>::Result >::Result
            >,
		public Fixed4<
			typename _if< Conversion<typename Element< TListPos, 3,true>::Result,VariablePosBase>::exists,
			NullType,
			typename Element< TListPos, 3,true>::Result >::Result
			>


    {
        protected:
            typedef typename _if< Conversion<typename Element< TListPos, 0,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 0,true>::Result >::Result TArg1;
            typedef typename _if< Conversion<typename Element< TListPos, 1,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 1,true>::Result >::Result TArg2;
            typedef typename _if< Conversion<typename Element< TListPos, 2,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 2,true>::Result >::Result TArg3;
			typedef typename _if< Conversion<typename Element< TListPos, 3,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 3,true>::Result >::Result TArg4;

//            typedef typename TypeTraits< typename TypeTraits<typename Element< TListPos, 0,true>::Result>::ParameterType>::NoConst TArg1Ref;
//            typedef typename TypeTraits< typename TypeTraits<typename Element< TListPos, 1,true>::Result>::ParameterType>::NoConst TArg2Ref;
//            typedef typename TypeTraits< typename TypeTraits<typename Element< TListPos, 2,true>::Result>::ParameterType>::NoConst TArg3Ref;
//			typedef typename TypeTraits< typename TypeTraits<typename Element< TListPos, 3,true>::Result>::ParameterType>::NoConst TArg4Ref;
            typedef typename TypeTraits<typename Element< TListPos, 0,true>::Result>::ParameterType TArg1Ref;
            typedef typename TypeTraits<typename Element< TListPos, 1,true>::Result>::ParameterType TArg2Ref;
            typedef typename TypeTraits<typename Element< TListPos, 2,true>::Result>::ParameterType TArg3Ref;
			typedef typename TypeTraits<typename Element< TListPos, 3,true>::Result>::ParameterType TArg4Ref;


        Linker_Base( TArg1Ref arg1,
                    TArg2Ref arg2,
                    TArg3Ref arg3,TArg4Ref arg4 ):Fixed1<TArg1>( arg1 ),Fixed2<TArg2>( arg2 ),Fixed3<TArg3>( arg3 ),Fixed4<TArg4>( arg4 ){}
		bool operator == ( const Linker_Base<TListPos,4>& ob2 ) const
		{
			return mpl::equal<true>( *((Fixed1<TArg1>*)this),ob2 ) && 
				mpl::equal<true>( *((Fixed2<TArg2>*)this),ob2 ) &&
				mpl::equal<true>( *((Fixed3<TArg3>*)this),ob2 ) &&
				mpl::equal<true>( *((Fixed4<TArg4>*)this),ob2 );
		}
    };

	template <class TListPos> class Linker_Base< TListPos,5> :
        public Fixed1<
            typename _if< Conversion<typename Element< TListPos, 0,true>::Result,VariablePosBase>::exists,
            NullType,
            typename Element< TListPos, 0,true>::Result >::Result
            >,
        public Fixed2<
            typename _if< Conversion<typename Element< TListPos, 1,true>::Result,VariablePosBase>::exists,
            NullType,
            typename Element< TListPos, 1,true>::Result >::Result
            >,
        public Fixed3<
            typename _if< Conversion<typename Element< TListPos, 2,true>::Result,VariablePosBase>::exists,
            NullType,
            typename Element< TListPos, 2,true>::Result >::Result
            >,
		public Fixed4<
			typename _if< Conversion<typename Element< TListPos, 3,true>::Result,VariablePosBase>::exists,
			NullType,
			typename Element< TListPos, 3,true>::Result >::Result
			>,
		public Fixed5<
			typename _if< Conversion<typename Element< TListPos, 4,true>::Result,VariablePosBase>::exists,
			NullType,
			typename Element< TListPos, 4,true>::Result >::Result
			>


    {
        protected:
            typedef typename _if< Conversion<typename Element< TListPos, 0,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 0,true>::Result >::Result TArg1;
            typedef typename _if< Conversion<typename Element< TListPos, 1,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 1,true>::Result >::Result TArg2;
            typedef typename _if< Conversion<typename Element< TListPos, 2,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 2,true>::Result >::Result TArg3;
			typedef typename _if< Conversion<typename Element< TListPos, 3,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 3,true>::Result >::Result TArg4;
			typedef typename _if< Conversion<typename Element< TListPos, 4,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 4,true>::Result >::Result TArg5;

/*
            typedef typename TypeTraits< typename TypeTraits<typename Element< TListPos, 0,true>::Result>::ParameterType>::NoConst TArg1Ref;
            typedef typename TypeTraits< typename TypeTraits<typename Element< TListPos, 1,true>::Result>::ParameterType>::NoConst TArg2Ref;
            typedef typename TypeTraits< typename TypeTraits<typename Element< TListPos, 2,true>::Result>::ParameterType>::NoConst TArg3Ref;
			typedef typename TypeTraits< typename TypeTraits<typename Element< TListPos, 3,true>::Result>::ParameterType>::NoConst TArg4Ref;
			typedef typename TypeTraits< typename TypeTraits<typename Element< TListPos, 4,true>::Result>::ParameterType>::NoConst TArg5Ref;
*/
            typedef typename TypeTraits<typename Element< TListPos, 0,true>::Result>::ParameterType TArg1Ref;
            typedef typename TypeTraits<typename Element< TListPos, 1,true>::Result>::ParameterType TArg2Ref;
            typedef typename TypeTraits<typename Element< TListPos, 2,true>::Result>::ParameterType TArg3Ref;
			typedef typename TypeTraits<typename Element< TListPos, 3,true>::Result>::ParameterType TArg4Ref;
			typedef typename TypeTraits<typename Element< TListPos, 4,true>::Result>::ParameterType TArg5Ref;
        Linker_Base( TArg1Ref arg1,
                    TArg2Ref arg2,
                    TArg3Ref arg3,TArg4Ref arg4,TArg5Ref arg5 ):Fixed1<TArg1>( arg1 ),Fixed2<TArg2>( arg2 ),Fixed3<TArg3>( arg3 ),Fixed4<TArg4>( arg4 ),Fixed5<TArg5>( arg5 ){}

		bool operator == ( const Linker_Base<TListPos,5>& ob2 ) const
		{
			return mpl::equal<true>( *((Fixed1<TArg1>*)this),ob2 ) && 
				mpl::equal<true>( *((Fixed2<TArg2>*)this),ob2 ) &&
				mpl::equal<true>( *((Fixed3<TArg3>*)this),ob2 ) &&
				mpl::equal<true>( *((Fixed4<TArg4>*)this),ob2 ) &&
				mpl::equal<true>( *((Fixed5<TArg5>*)this),ob2 );
		}

    };


	template <class TListPos> class Linker_Base< TListPos,6> :
        public Fixed1<
            typename _if< Conversion<typename Element< TListPos, 0,true>::Result,VariablePosBase>::exists,
            NullType,
            typename Element< TListPos, 0,true>::Result >::Result
            >,
        public Fixed2<
            typename _if< Conversion<typename Element< TListPos, 1,true>::Result,VariablePosBase>::exists,
            NullType,
            typename Element< TListPos, 1,true>::Result >::Result
            >,
        public Fixed3<
            typename _if< Conversion<typename Element< TListPos, 2,true>::Result,VariablePosBase>::exists,
            NullType,
            typename Element< TListPos, 2,true>::Result >::Result
            >,
		public Fixed4<
			typename _if< Conversion<typename Element< TListPos, 3,true>::Result,VariablePosBase>::exists,
			NullType,
			typename Element< TListPos, 3,true>::Result >::Result
			>,
		public Fixed5<
			typename _if< Conversion<typename Element< TListPos, 4,true>::Result,VariablePosBase>::exists,
			NullType,
			typename Element< TListPos, 4,true>::Result >::Result
			>,
		public Fixed6<
		typename _if< Conversion<typename Element< TListPos, 5,true>::Result,VariablePosBase>::exists,
		NullType,
		typename Element< TListPos, 5,true>::Result >::Result
		>


    {
        protected:
            typedef typename _if< Conversion<typename Element< TListPos, 0,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 0,true>::Result >::Result TArg1;
            typedef typename _if< Conversion<typename Element< TListPos, 1,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 1,true>::Result >::Result TArg2;
            typedef typename _if< Conversion<typename Element< TListPos, 2,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 2,true>::Result >::Result TArg3;
			typedef typename _if< Conversion<typename Element< TListPos, 3,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 3,true>::Result >::Result TArg4;
			typedef typename _if< Conversion<typename Element< TListPos, 4,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 4,true>::Result >::Result TArg5;
			typedef typename _if< Conversion<typename Element< TListPos, 5,true>::Result,VariablePosBase>::exists,NullType,typename Element< TListPos, 5,true>::Result >::Result TArg6;

            //typedef typename TypeTraits< typename TypeTraits<typename Element< TListPos, 0,true>::Result>::ParameterType>::NoConst TArg1Ref;
            //typedef typename TypeTraits< typename TypeTraits<typename Element< TListPos, 1,true>::Result>::ParameterType>::NoConst TArg2Ref;
            //typedef typename TypeTraits< typename TypeTraits<typename Element< TListPos, 2,true>::Result>::ParameterType>::NoConst TArg3Ref;
			//typedef typename TypeTraits< typename TypeTraits<typename Element< TListPos, 3,true>::Result>::ParameterType>::NoConst TArg4Ref;
			//typedef typename TypeTraits< typename TypeTraits<typename Element< TListPos, 4,true>::Result>::ParameterType>::NoConst TArg5Ref;
			//typedef typename TypeTraits< typename TypeTraits<typename Element< TListPos, 5,true>::Result>::ParameterType>::NoConst TArg6Ref;

            typedef typename TypeTraits<typename Element< TListPos, 0,true>::Result>::ParameterType TArg1Ref;
            typedef typename TypeTraits<typename Element< TListPos, 1,true>::Result>::ParameterType TArg2Ref;
            typedef typename TypeTraits<typename Element< TListPos, 2,true>::Result>::ParameterType TArg3Ref;
			typedef typename TypeTraits<typename Element< TListPos, 3,true>::Result>::ParameterType TArg4Ref;
			typedef typename TypeTraits<typename Element< TListPos, 4,true>::Result>::ParameterType TArg5Ref;
			typedef typename TypeTraits<typename Element< TListPos, 5,true>::Result>::ParameterType TArg6Ref;
        Linker_Base( TArg1Ref arg1,
                    TArg2Ref arg2,
                    TArg3Ref arg3,TArg4Ref arg4,TArg5Ref arg5,TArg6Ref arg6):Fixed1<TArg1>( arg1 ),
						Fixed2<TArg2>( arg2 ),Fixed3<TArg3>( arg3 ),Fixed4<TArg4>( arg4 ),
						Fixed5<TArg5>( arg5 ),Fixed6<TArg6>( arg6 ){}

		bool operator == ( const Linker_Base<TListPos,6>& ob2 ) const
		{
			return mpl::equal<true>( *((Fixed1<TArg1>*)this),ob2 ) && 
				mpl::equal<true>( *((Fixed2<TArg2>*)this),ob2 ) &&
				mpl::equal<true>( *((Fixed3<TArg3>*)this),ob2 ) &&
				mpl::equal<true>( *((Fixed4<TArg4>*)this),ob2 ) &&
				mpl::equal<true>( *((Fixed5<TArg5>*)this),ob2 ) &&
				mpl::equal<true>( *((Fixed6<TArg6>*)this),ob2 );
		}

    };
    template <class F,class TRet,
            class TListVariable, class TListPos> class Linker : public Linker_Base<TListPos,Length< TListPos >::result>
    {
        private:

            typedef Linker_Base< TListPos,Length< TListPos >::result> BaseClass;
            //typedef Fixed1< typename BaseClass::TArg1 > TFixed1;


            typedef typename TypeTraits< typename Element< TListVariable, 0,true>::Result>::ParameterType TV1;
            typedef typename TypeTraits< typename Element< TListVariable, 1,true>::Result>::ParameterType TV2;
            typedef typename TypeTraits< typename Element< TListVariable, 2,true>::Result>::ParameterType TV3;
			typedef typename TypeTraits< typename Element< TListVariable, 3,true>::Result>::ParameterType TV4;
			typedef typename TypeTraits< typename Element< TListVariable, 4,true>::Result>::ParameterType TV5;
			typedef typename TypeTraits< typename Element< TListVariable, 5,true>::Result>::ParameterType TV6;

            typedef typename Element< TListPos, 0,true>::Result TP1;
            typedef typename Element< TListPos, 1,true>::Result TP2;
            typedef typename Element< TListPos, 2,true>::Result TP3;
            typedef typename Element< TListPos, 3,true>::Result TP4;
            typedef typename Element< TListPos, 4,true>::Result TP5;
            typedef typename Element< TListPos, 5,true>::Result TP6;



            //typedef typename TypeTraits< typename TypeTraits<TP1>::ParameterType>::NoConst TP1Ref;
            /*typedef typename TypeTraits< typename TypeTraits<TP2>::ParameterType>::NoConst TP2Ref;
            typedef typename TypeTraits< typename TypeTraits<TP3>::ParameterType>::NoConst TP3Ref;
            typedef typename TypeTraits< typename TypeTraits<TP4>::ParameterType>::NoConst TP4Ref;
            typedef typename TypeTraits< typename TypeTraits<TP5>::ParameterType>::NoConst TP5Ref;
            typedef typename TypeTraits< typename TypeTraits<TP6>::ParameterType>::NoConst TP6Ref;*/

            typedef typename TypeTraits<TP1>::ParameterType TP1Ref;
            typedef typename TypeTraits<TP2>::ParameterType TP2Ref;
            typedef typename TypeTraits<TP3>::ParameterType TP3Ref;
            typedef typename TypeTraits<TP4>::ParameterType TP4Ref;
            typedef typename TypeTraits<TP5>::ParameterType TP5Ref;
            typedef typename TypeTraits<TP6>::ParameterType TP6Ref;

            enum {totallength = Length< TListPos >::result};



        public:
                Linker( const F& functor ): mFunctor( functor )
                {
                }
                Linker( const F& functor,TP1Ref arg1 ): BaseClass( arg1 ),mFunctor( functor )
                {
                }
                Linker( const F& functor,TP1Ref arg1,TP2Ref arg2 ): BaseClass( arg1,arg2 ),mFunctor( functor )
                {
                }
                Linker( const F& functor,TP1Ref arg1,TP2Ref arg2,TP3Ref arg3 ): BaseClass( arg1,arg2,arg3 ),mFunctor( functor )
                {
                }
				Linker( const F& functor,TP1Ref arg1,TP2Ref arg2,TP3Ref arg3,TP4Ref arg4 ): BaseClass( arg1,arg2,arg3,arg4 ),mFunctor( functor )
				{
				}
				Linker( const F& functor,TP1Ref arg1,TP2Ref arg2,TP3Ref arg3,TP4Ref arg4,TP5Ref arg5 ): BaseClass( arg1,arg2,arg3,arg4,arg5 ),mFunctor( functor )
				{
				}
				Linker( const F& functor,TP1Ref arg1,TP2Ref arg2,TP3Ref arg3,TP4Ref arg4,TP5Ref arg5,TP6Ref arg6): BaseClass( arg1,arg2,arg3,arg4,arg5,arg6 ),mFunctor( functor )
				{
				}

				template <class F2>
				bool operator == ( const F2& ob2 ) const
				{
					return _equality( Int2Type< Conversion<F2, Linker<F,TRet,TListVariable,TListPos> >::exists >(),ob2 );
				}


                TRet operator()( )
                {
                    return callfunction( Int2Type<totallength>() );
                }

                TRet operator()( TV1 arg1)
                {
                    return callfunction( Int2Type<totallength>(),arg1 );
                }
                TRet operator()( TV1 arg1, TV2 arg2 )
                {
                    return callfunction( Int2Type<totallength>(),arg1,arg2 );
                }
                TRet operator()( TV1 arg1,TV2 arg2,TV3 arg3 )
                {
                    return callfunction( Int2Type<totallength>(),arg1,arg2,arg3 );
                }
				TRet operator()( TV1 arg1,TV2 arg2,TV3 arg3,TV4 arg4 )
				{
					return callfunction( Int2Type<totallength>(),arg1,arg2,arg3,arg4 );
				}
				TRet operator()( TV1 arg1,TV2 arg2,TV3 arg3,TV4 arg4,TV5 arg5 )
				{
					return callfunction( Int2Type<totallength>(),arg1,arg2,arg3,arg4,arg5 );
				}
				TRet operator()( TV1 arg1,TV2 arg2,TV3 arg3,TV4 arg4,TV5 arg5,TV6 arg6 )
				{
					return callfunction( Int2Type<totallength>(),arg1,arg2,arg3,arg4,arg5,arg6 );
				}



				typedef typename _if< Conversion<TP1,VariablePosBase>::exists,TP1,Int2Type<1> >::Result FirstType;
                typedef typename _if< Conversion<TP2,VariablePosBase>::exists,TP2,Int2Type<2> >::Result SecondType;
                typedef typename _if< Conversion<TP3,VariablePosBase>::exists,TP3,Int2Type<3> >::Result ThirdType;
                typedef typename _if< Conversion<TP4,VariablePosBase>::exists,TP4,Int2Type<4> >::Result FourthType;
                typedef typename _if< Conversion<TP5,VariablePosBase>::exists,TP5,Int2Type<5> >::Result FifthType;
                typedef typename _if< Conversion<TP6,VariablePosBase>::exists,TP6,Int2Type<6> >::Result SixthType;



        private:
            F mFunctor;

            template <class TListaArgs,class T> struct GetArg;
            template < class TListaArgs > struct GetArgBase
            {

                typedef typename TypeTraits< typename Element< TListaArgs,0,false>::Result >::ParameterType Arg1Ref;
                typedef typename TypeTraits< typename Element< TListaArgs,1,false>::Result >::ParameterType Arg2Ref;
                typedef typename TypeTraits< typename Element< TListaArgs,2,false>::Result >::ParameterType Arg3Ref;
				typedef typename TypeTraits< typename Element< TListaArgs,3,false>::Result >::ParameterType Arg4Ref;
				typedef typename TypeTraits< typename Element< TListaArgs,4,false>::Result >::ParameterType Arg5Ref;
				typedef typename TypeTraits< typename Element< TListaArgs,5,false>::Result >::ParameterType Arg6Ref;

            };
            template <class TListaArgs> struct GetArg< TListaArgs,Int2Type<1> > : public GetArgBase<TListaArgs>
            {
               static inline TP1Ref get( typename GetArgBase<TListaArgs>::Arg1Ref, typename GetArgBase<TListaArgs>::Arg2Ref,typename GetArgBase<TListaArgs>::Arg3Ref,
								typename GetArgBase<TListaArgs>::Arg4Ref, typename GetArgBase<TListaArgs>::Arg5Ref,typename GetArgBase<TListaArgs>::Arg6Ref,
                                 BaseClass& fixed )
               {
                   return fixed.f1;
               }
            };
            template <class TListaArgs> struct GetArg< TListaArgs,Int2Type<2> >: public GetArgBase<TListaArgs>
            {
               static inline TP2Ref get( typename GetArgBase<TListaArgs>::Arg1Ref, typename GetArgBase<TListaArgs>::Arg2Ref,typename GetArgBase<TListaArgs>::Arg3Ref,
								typename GetArgBase<TListaArgs>::Arg4Ref, typename GetArgBase<TListaArgs>::Arg5Ref,typename GetArgBase<TListaArgs>::Arg6Ref,
								BaseClass& fixed)
               {
                   return fixed.f2;
               }
            };
            template <class TListaArgs> struct GetArg< TListaArgs,Int2Type<3> >: public GetArgBase<TListaArgs>
            {
               static inline TP3Ref get( typename GetArgBase<TListaArgs>::Arg1Ref, typename GetArgBase<TListaArgs>::Arg2Ref,typename GetArgBase<TListaArgs>::Arg3Ref,
								typename GetArgBase<TListaArgs>::Arg4Ref, typename GetArgBase<TListaArgs>::Arg5Ref,typename GetArgBase<TListaArgs>::Arg6Ref,
                                 BaseClass& fixed)
               {
                   return fixed.f3;
               }
            };
			template <class TListaArgs> struct GetArg< TListaArgs,Int2Type<4> >: public GetArgBase<TListaArgs>
			{
				static inline TP4Ref get( typename GetArgBase<TListaArgs>::Arg1Ref, typename GetArgBase<TListaArgs>::Arg2Ref,typename GetArgBase<TListaArgs>::Arg3Ref,
					typename GetArgBase<TListaArgs>::Arg4Ref, typename GetArgBase<TListaArgs>::Arg5Ref,typename GetArgBase<TListaArgs>::Arg6Ref,
					BaseClass& fixed)
				{
					return fixed.f4;
				}
			};
			template <class TListaArgs> struct GetArg< TListaArgs,Int2Type<5> >: public GetArgBase<TListaArgs>
			{
				static inline TP5Ref get( typename GetArgBase<TListaArgs>::Arg1Ref, typename GetArgBase<TListaArgs>::Arg2Ref,typename GetArgBase<TListaArgs>::Arg3Ref,
					typename GetArgBase<TListaArgs>::Arg4Ref, typename GetArgBase<TListaArgs>::Arg5Ref,typename GetArgBase<TListaArgs>::Arg6Ref,
					BaseClass& fixed)
				{
					return fixed.f5;
				}
			};
			template <class TListaArgs> struct GetArg< TListaArgs,Int2Type<6> >: public GetArgBase<TListaArgs>
			{
				static inline TP6Ref get( typename GetArgBase<TListaArgs>::Arg1Ref, typename GetArgBase<TListaArgs>::Arg2Ref,typename GetArgBase<TListaArgs>::Arg3Ref,
					typename GetArgBase<TListaArgs>::Arg4Ref, typename GetArgBase<TListaArgs>::Arg5Ref,typename GetArgBase<TListaArgs>::Arg6Ref,
					BaseClass& fixed)
				{
					return fixed.f6;
				}
			};
            template <class TListaArgs> struct GetArg< TListaArgs,VariablePos<1> > :public GetArgBase<TListaArgs>
            {
                static inline TV1 get( typename GetArgBase<TListaArgs>::Arg1Ref arg1, typename GetArgBase<TListaArgs>::Arg2Ref,typename GetArgBase<TListaArgs>::Arg3Ref,
							typename GetArgBase<TListaArgs>::Arg4Ref, typename GetArgBase<TListaArgs>::Arg5Ref,typename GetArgBase<TListaArgs>::Arg6Ref,
                              BaseClass&)
                {

                    return arg1;
                }
            };
            template <class TListaArgs> struct GetArg< TListaArgs,VariablePos<2> > :public GetArgBase<TListaArgs>
            {
                static inline TV2 get( typename GetArgBase<TListaArgs>::Arg1Ref, typename GetArgBase<TListaArgs>::Arg2Ref arg2,typename GetArgBase<TListaArgs>::Arg3Ref ,
							 typename GetArgBase<TListaArgs>::Arg4Ref, typename GetArgBase<TListaArgs>::Arg5Ref,typename GetArgBase<TListaArgs>::Arg6Ref,
                               BaseClass&)
                {
                    return arg2;
                }
            };
            template <class TListaArgs> struct GetArg< TListaArgs,VariablePos<3> >: public GetArgBase<TListaArgs>
            {
                static inline TV3 get( typename GetArgBase<TListaArgs>::Arg1Ref, typename GetArgBase<TListaArgs>::Arg2Ref,typename GetArgBase<TListaArgs>::Arg3Ref arg3,
								typename GetArgBase<TListaArgs>::Arg4Ref, typename GetArgBase<TListaArgs>::Arg5Ref,typename GetArgBase<TListaArgs>::Arg6Ref,
								BaseClass&)
                {
                    return arg3;
                }
            };
			template <class TListaArgs> struct GetArg< TListaArgs,VariablePos<4> >: public GetArgBase<TListaArgs>
			{
				static inline TV4 get( typename GetArgBase<TListaArgs>::Arg1Ref, typename GetArgBase<TListaArgs>::Arg2Ref,typename GetArgBase<TListaArgs>::Arg3Ref arg3,
					typename GetArgBase<TListaArgs>::Arg4Ref arg4, typename GetArgBase<TListaArgs>::Arg5Ref,typename GetArgBase<TListaArgs>::Arg6Ref,
					BaseClass&)
				{
					return arg4;
				}
			};
			template <class TListaArgs> struct GetArg< TListaArgs,VariablePos<5> >: public GetArgBase<TListaArgs>
			{
				static inline TV5 get( typename GetArgBase<TListaArgs>::Arg1Ref, typename GetArgBase<TListaArgs>::Arg2Ref,typename GetArgBase<TListaArgs>::Arg3Ref arg3,
					typename GetArgBase<TListaArgs>::Arg4Ref, typename GetArgBase<TListaArgs>::Arg5Ref arg5,typename GetArgBase<TListaArgs>::Arg6Ref,
					BaseClass&)
				{
					return arg5;
				}
			};
			template <class TListaArgs> struct GetArg< TListaArgs,VariablePos<6> >: public GetArgBase<TListaArgs>
			{
				static inline TV6 get( typename GetArgBase<TListaArgs>::Arg1Ref, typename GetArgBase<TListaArgs>::Arg2Ref,typename GetArgBase<TListaArgs>::Arg3Ref arg3,
					typename GetArgBase<TListaArgs>::Arg4Ref, typename GetArgBase<TListaArgs>::Arg5Ref,typename GetArgBase<TListaArgs>::Arg6Ref arg6,
					BaseClass&)
				{
					return arg6;
				}
			};

            //one  arg
            inline TRet callfunction( Int2Type<1> )
            {
                //return mFunctor( mFixed.f1 ); //si no hay parametros de entrada a la fuerza es uno fijo
                //return mFunctor(TFixed1::f1);
                return mFunctor(this->f1);

            }
            inline TRet callfunction( Int2Type<1>,TV1 arg1)
            {
                return mFunctor( arg1  );
            }
            //total length == 2
			inline TRet callfunction( Int2Type<2> )
			{
				return mFunctor(this->f1,this->f2);
			}

            inline TRet callfunction( Int2Type<2>,TV1 arg1)
            {

                return mFunctor(  GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),FirstType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this),
                    GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),SecondType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this));

            }

            inline TRet callfunction( Int2Type<2>,TV1 arg1,TV2 arg2)
            {
                //MPL_STATIC_ASSERT( (isSame<FirstType,VariablePos<1> >::result), CACA );
                return mFunctor(  GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),FirstType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this),
                    GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),SecondType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this));

            }
             //total length == 3
			inline TRet callfunction( Int2Type<3> )
			{
				return mFunctor(this->f1,this->f2,this->f3);
			}

            inline TRet callfunction( Int2Type<3>,TV1 arg1)
            {

              return mFunctor(  GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),FirstType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this),
                    GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),SecondType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this),
                    GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),ThirdType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this));


            }
            inline TRet callfunction( Int2Type<3>,TV1 arg1,TV2 arg2)
            {

                return mFunctor(  GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),FirstType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this),
                    GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),SecondType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this),
                    GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),ThirdType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this));

            }
            inline TRet callfunction( Int2Type<3>,TV1 arg1,TV2 arg2,TV3 arg3)
            {
               return mFunctor(  GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),FirstType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this),
                    GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),SecondType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this),
                    GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),ThirdType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this));
            }
			//total length == 4
			inline TRet callfunction( Int2Type<4> )
			{
				return mFunctor(this->f1,this->f2,this->f3,this->f4);
			}

			inline TRet callfunction( Int2Type<4>,TV1 arg1 )
			{
				return mFunctor(  GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),FirstType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),SecondType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),ThirdType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),FourthType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this)
					);
			}
			inline TRet callfunction( Int2Type<4>,TV1 arg1,TV2 arg2 )
			{
				return mFunctor(  GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),FirstType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),SecondType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),ThirdType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),FourthType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this)
					);
			}
			inline TRet callfunction( Int2Type<4>,TV1 arg1,TV2 arg2,TV3 arg3 )
			{
				return mFunctor(  GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),FirstType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),SecondType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),ThirdType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),FourthType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this)
					);
			}
			inline TRet callfunction( Int2Type<4>,TV1 arg1,TV2 arg2,TV3 arg3,TV4 arg4 )
			{
				return mFunctor(  GetArg< TYPELIST(TV1,TV2,TV3,TV4,NullType,NullType),FirstType>::get(arg1,arg2,arg3,arg4,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,NullType,NullType),SecondType>::get(arg1,arg2,arg3,arg4,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,NullType,NullType),ThirdType>::get(arg1,arg2,arg3,arg4,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,NullType,NullType),FourthType>::get(arg1,arg2,arg3,arg4,nullType,nullType,*this)
					);
			}
			//total length == 5
			inline TRet callfunction( Int2Type<5> )
			{
				return mFunctor(this->f1,this->f2,this->f3,this->f4,this->f5);
			}

			inline TRet callfunction( Int2Type<5>,TV1 arg1 )
			{
				return mFunctor(  GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),FirstType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),SecondType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),ThirdType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),FourthType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),FifthType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this)
					);
			}
			inline TRet callfunction( Int2Type<5>,TV1 arg1,TV2 arg2 )
			{
				return mFunctor(  GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),FirstType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),SecondType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),ThirdType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),FourthType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),FifthType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this)
					);
			}
			inline TRet callfunction( Int2Type<5>,TV1 arg1,TV2 arg2, TV3 arg3 )
			{
				return mFunctor(  GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),FirstType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),SecondType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),ThirdType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),FourthType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),FifthType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this)
					);
			}
			inline TRet callfunction( Int2Type<5>,TV1 arg1,TV2 arg2, TV3 arg3,TV4 arg4 )
			{
				return mFunctor(  GetArg< TYPELIST(TV1,TV2,TV3,TV4,NullType,NullType),FirstType>::get(arg1,arg2,arg3,arg4,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,NullType,NullType),SecondType>::get(arg1,arg2,arg3,arg4,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,NullType,NullType),ThirdType>::get(arg1,arg2,arg3,arg4,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,NullType,NullType),FourthType>::get(arg1,arg2,arg3,arg4,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,NullType,NullType),FifthType>::get(arg1,arg2,arg3,arg4,nullType,nullType,*this)
					);
			}
			inline TRet callfunction( Int2Type<5>,TV1 arg1,TV2 arg2, TV3 arg3,TV4 arg4,TV5 arg5 )
			{
				return mFunctor(  GetArg< TYPELIST(TV1,TV2,TV3,TV4,TV5,NullType),FirstType>::get(arg1,arg2,arg3,arg4,arg5,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,TV5,NullType),SecondType>::get(arg1,arg2,arg3,arg4,arg5,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,TV5,NullType),ThirdType>::get(arg1,arg2,arg3,arg4,arg5,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,TV5,NullType),FourthType>::get(arg1,arg2,arg3,arg4,arg5,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,TV5,NullType),FifthType>::get(arg1,arg2,arg3,arg4,arg5,nullType,*this)
					);
			}

			//total length == 6
			inline TRet callfunction( Int2Type<6> )
			{
				return mFunctor(this->f1,this->f2,this->f3,this->f4,this->f5,this->f6);
			}

			inline TRet callfunction( Int2Type<6>,TV1 arg1 )
			{
				return mFunctor(  GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),FirstType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),SecondType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),ThirdType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),FourthType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),FifthType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,NullType,NullType,NullType,NullType,NullType),SixthType>::get(arg1,nullType,nullType,nullType,nullType,nullType,*this)
					);
			}

			inline TRet callfunction( Int2Type<6>,TV1 arg1,TV2 arg2 )
			{
				return mFunctor(  GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),FirstType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),SecondType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),ThirdType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),FourthType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),FifthType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,NullType,NullType,NullType,NullType),SixthType>::get(arg1,arg2,nullType,nullType,nullType,nullType,*this)
					);
			}
			inline TRet callfunction( Int2Type<6>,TV1 arg1,TV2 arg2, TV3 arg3 )
			{
				return mFunctor(  GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),FirstType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),SecondType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),ThirdType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),FourthType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),FifthType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,NullType,NullType,NullType),SixthType>::get(arg1,arg2,arg3,nullType,nullType,nullType,*this)
					);
			}
			inline TRet callfunction( Int2Type<6>,TV1 arg1,TV2 arg2, TV3 arg3,TV4 arg4 )
			{
				return mFunctor(  GetArg< TYPELIST(TV1,TV2,TV3,TV4,NullType,NullType),FirstType>::get(arg1,arg2,arg3,arg4,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,NullType,NullType),SecondType>::get(arg1,arg2,arg3,arg4,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,NullType,NullType),ThirdType>::get(arg1,arg2,arg3,arg4,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,NullType,NullType),FourthType>::get(arg1,arg2,arg3,arg4,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,NullType,NullType),FifthType>::get(arg1,arg2,arg3,arg4,nullType,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,NullType,NullType),SixthType>::get(arg1,arg2,arg3,arg4,nullType,nullType,*this)
					);
			}

			inline TRet callfunction( Int2Type<6>,TV1 arg1,TV2 arg2, TV3 arg3,TV4 arg4,TV5 arg5 )
			{
				return mFunctor(  GetArg< TYPELIST(TV1,TV2,TV3,TV4,TV5,NullType),FirstType>::get(arg1,arg2,arg3,arg4,arg5,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,TV5,NullType),SecondType>::get(arg1,arg2,arg3,arg4,arg5,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,TV5,NullType),ThirdType>::get(arg1,arg2,arg3,arg4,arg5,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,TV5,NullType),FourthType>::get(arg1,arg2,arg3,arg4,arg5,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,TV5,NullType),FifthType>::get(arg1,arg2,arg3,arg4,arg5,nullType,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,TV5,NullType),SixthType>::get(arg1,arg2,arg3,arg4,arg5,nullType,*this)
					);
			}
			inline TRet callfunction( Int2Type<6>,TV1 arg1,TV2 arg2, TV3 arg3,TV4 arg4,TV5 arg5,TV4 arg6 )
			{
				return mFunctor(  GetArg< TYPELIST(TV1,TV2,TV3,TV4,TV5,TV6),FirstType>::get(arg1,arg2,arg3,arg4,arg5,arg6,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,TV5,TV6),SecondType>::get(arg1,arg2,arg3,arg4,arg5,arg6,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,TV5,TV6),ThirdType>::get(arg1,arg2,arg3,arg4,arg5,arg6,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,TV5,TV6),FourthType>::get(arg1,arg2,arg3,arg4,arg5,arg6,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,TV5,TV6),FifthType>::get(arg1,arg2,arg3,arg4,arg5,arg6,*this),
					GetArg< TYPELIST(TV1,TV2,TV3,TV4,TV5,TV6),SixthType>::get(arg1,arg2,arg3,arg4,arg5,arg6,*this)
					);
			}


			template <class U>
			bool _equality( Int2Type<false>,const U&) const
			{
				return false;
			}
			bool _equality( Int2Type<true >,const Linker<F,TRet,TListVariable,TListPos>& ob2) const
			{
				const Linker_Base<TListPos,totallength>& obAux = ob2;
				return ::mpl::equal<true>( mFunctor, ob2.mFunctor )
					&& 
					Linker_Base<TListPos,totallength>::operator ==( obAux );
				
					//Linker_Base<TListPos,totallength>::operator ==( (const Linker_Base<TListPos,totallength>&)ob2 );
			}


 };

        template <class TRet,class TVariableList,class Arg1, class T>
          Linker< T,TRet,TVariableList,TYPELIST(Arg1)> linkFunctor( T functor,Arg1 arg1)
        {
            return Linker< T,TRet,TVariableList,TYPELIST(Arg1) >( functor,arg1 );

        }
        template <class TRet,class TVariableList,class Arg1, class Arg2,class T>
          Linker< T,TRet,TVariableList,TYPELIST(Arg1,Arg2)> linkFunctor( T functor,Arg1 arg1, Arg2 arg2)
        {
            return Linker< T,TRet,TVariableList,TYPELIST(Arg1,Arg2) >( functor,arg1,arg2 );
        }

        template <class TRet,class TVariableList,class Arg1,class Arg2,class Arg3,class T>
          Linker< T,TRet,TVariableList,TYPELIST(Arg1,Arg2,Arg3)> linkFunctor( T functor,Arg1 arg1,Arg2 arg2,Arg3 arg3)
        {
            return Linker< T,TRet,TVariableList,TYPELIST(Arg1,Arg2,Arg3) >( functor,arg1,arg2,arg3 );

        }
		  template <class TRet,class TVariableList,class Arg1,class Arg2,class Arg3,class Arg4,class T>
		  Linker< T,TRet,TVariableList,TYPELIST(Arg1,Arg2,Arg3,Arg4)> linkFunctor( T functor,Arg1 arg1,Arg2 arg2,Arg3 arg3,Arg4 arg4)
		  {
			  return Linker< T,TRet,TVariableList,TYPELIST(Arg1,Arg2,Arg3,Arg4) >( functor,arg1,arg2,arg3,arg4 );

		  }
		  template <class TRet,class TVariableList,class Arg1,class Arg2,class Arg3,class Arg4,class Arg5,class T>
		  Linker< T,TRet,TVariableList,TYPELIST(Arg1,Arg2,Arg3,Arg4,Arg5)> linkFunctor( T functor,Arg1 arg1,Arg2 arg2,Arg3 arg3,Arg4 arg4,Arg5 arg5)
		  {
			  return Linker< T,TRet,TVariableList,TYPELIST(Arg1,Arg2,Arg3,Arg4,Arg5) >( functor,arg1,arg2,arg3,arg4,arg5 );

		  }
		  template <class TRet,class TVariableList,class Arg1,class Arg2,class Arg3,class Arg4,class Arg5,class Arg6,class T>
		  Linker< T,TRet,TVariableList,TYPELIST(Arg1,Arg2,Arg3,Arg4,Arg5,Arg6)> linkFunctor( T functor,Arg1 arg1,Arg2 arg2,Arg3 arg3,Arg4 arg4,Arg5 arg5, Arg6 arg6)
		  {

			  return Linker< T,TRet,TVariableList,TYPELIST(Arg1,Arg2,Arg3,Arg4,Arg5,Arg6) >( functor,arg1,arg2,arg3,arg4,arg5,arg6 );

		  }

/*problema TVariableList solo representa la parte no-fija,es decir, en lo que queda la funcion,
 y por tanto no equivale al V1,V2,..
*/
//TODO la idea es después que donde en TListVariable haya un NullType se ignore
   /*     template <class TRet,class Arg1, class Arg2>
          Linker< TRet (*)(typename _if< Conversion<Arg1,VariablePosBase>::exists,NullType,Arg1>::Result
                           ,typename _if< Conversion<Arg2,VariablePosBase>::exists,NullType,Arg2>::Result)
                           ,TRet,TYPELIST( (typename _if< Conversion<Arg1,VariablePosBase>::exists,NullType,Arg1>::Result)
                                         ,(typename _if< Conversion<Arg2,VariablePosBase>::exists,NullType,Arg2>::Result)
                                          ),
                                         TYPELIST(Arg1,Arg2)> link( TRet (*f)(typename _if< Conversion<Arg1,VariablePosBase>::exists,NullType,Arg1>::Result,
                                                                            typename _if< Conversion<Arg2,VariablePosBase>::exists,NullType,Arg2>::Result),Arg1 arg1, Arg2 arg2)
        {
         //   return Linker< TRet (*)(V1,V2),TRet,TYPELIST(V1,V2),TYPELIST(Arg1,Arg2) >( f,arg1,arg2 );
        }
        */
     /*   template <class TRet,class Arg1, class Arg2>
          Linker< TRet (*)(_if< Conversion<Arg1,VariablePosBase>::exists,NullType,Arg1>::Result
                           ,_if< Conversion<Arg2,VariablePosBase>::exists,NullType,Arg2>::Result),TRet,TYPELIST(V1,V2),TYPELIST(Arg1,Arg2)> link( TRet (*f)(V1,V2),Arg1 arg1, Arg2 arg2)
        {
            return Linker< TRet (*)(V1,V2),TRet,TYPELIST(V1,V2),TYPELIST(Arg1,Arg2) >( f,arg1,arg2 );
        }
        */



}

