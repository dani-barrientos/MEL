#pragma once
#include <mpl/typelist/TypeList.h>

namespace mpl
{
	namespace typelist
	{
		//length
		template <class TList > struct Length;
		template <> struct Length<NullType>
		{
			enum {result = 0 };
		};
		template <class H,class T> struct Length< TypeList<H,T> >
		{
			enum {result = Length<T>::result + 1};
		};

	}
}