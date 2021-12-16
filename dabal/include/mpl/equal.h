#pragma once
/**
* @class equal
* Create functor for equality
* @todo para pruebas por ahora, hay que refinar el paso de parámetros
*/
#include <mpl/TypeTraits.h>
namespace mpl
{
	//don't use Compare class directcly
	template <bool,bool defaultValue>
	struct Compare
	{
		template <class T> inline static bool compare( const T& t1, const T& t2 )
		{
			return t1 == t2;
		}
	};
	//specialization when not operator== present. Return true
	template <bool defaultValue >
	struct Compare<false,defaultValue>
	{
		template <class T> inline static bool compare( const T& t1, const T& t2 )
		{
			return defaultValue; 
		}
	};
	template <bool defaultValue,class T> bool equal( const T& origin,const typename TypeTraits<T>::ParameterType value )
	{
		return Compare< ::mpl::ComparableTraits<T>::result,defaultValue >::compare( origin,value );
	}
}