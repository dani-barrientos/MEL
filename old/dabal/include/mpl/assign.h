#pragma once
#include <mpl/TypeTraits.h>
#include <mpl/_If.h>
#include <mpl/Int2Type.h>
using mpl::Int2Type;
#include <mpl/equal.h>

namespace mpl
{
	/**
	* @class Assign
	* asignacion de variables
	* @param[in] isFunctor. Considera p2 como un functor (sin parámetros)
	*/
	template <class T,class T2,bool isFunctor>
	class Assign
	{
	public:
		Assign( T& p1, typename _if< isFunctor && !TypeTraits<T2>::isPointer ,const T2&,typename TypeTraits<T2>::ParameterType >::Result p2) :
			mP1( p1 ),
			mP2( p2 )
		{
		}
		T& operator()( )
		{
			return _assign( mP1,mP2,_isfunctor );
		}
		template <class F2>
		bool operator == ( const F2& ob2 ) const
		{
			return equality( Int2Type< Conversion<F2, Assign<T,T2,isFunctor> >::exists >(),ob2 );
		}

	private:
		static Int2Type<isFunctor> _isfunctor;
		T& mP1;
		T2 mP2; //TODO no estoy seguro de si tendré que usar type_traits para usar o no referencia
		T& _assign( T& p1, typename TypeTraits<T2>::ParameterType p2,Int2Type<false> )
		{
			return p1 = p2;
		}
		T& _assign( T& p1, /*typename TypeTraits<typename TypeTraits<T2>::ParameterType>::NoConst*/ T2& p2,Int2Type<true> )
		{
			return p1 = p2();
		}

		template <class F>
		bool equality( Int2Type<false>,const F&) const
		{
			return false;
		}
		bool equality( Int2Type<true>,const Assign<T,T2,isFunctor> & a2) const
		{
			//return ( mP1 == a2.mP1 && mP2 == a2.mP2 );
			return ::mpl::equal<true>(mP1,a2.mP1) && ::mpl::equal<true>( mP2, a2.mP2 );
		}

	};
	template < bool isFunctor,class T,class T2 >
	Assign<T,T2,isFunctor> assign( T& p1,T2 p2/*typename TypeTraits<T2>::ParameterType p2*/ )
	{
		return Assign<T,T2,isFunctor>(p1,p2);
	}
	template<class T, class T2,bool isFunctor> Int2Type<isFunctor> Assign<T,T2,isFunctor>::_isfunctor = Int2Type<isFunctor>();
}
