#pragma once
#include <mpl/CommonTypes.h>
#include <mpl/TypeTraits.h>

namespace mpl
{
	/**
	* utility to see in compile-time if one class can be converted in another
	* usage: Conversion<T1,T2>::exists --> if T1 can be converted in T2
	*/
	template <class T, class U>
	struct Conversion
	{
	private:
		//TODO revisar. El tema es detectar si es tipo funcion ya que no se puede declarar un statico de ese tipo, por lo que hay que declarar otra cosa que de como resultado que no hay conversion	 
		static typename _if<TypeTraits<T>::isFunction,Big,typename TypeTraits<T>::ParameterType>::Result x;
		static Small Test( U );
		static Big Test(...);
		//static T MakeT();
	public:
		enum { exists =(sizeof( Test(x) )== sizeof(Small) ) };
	};
}