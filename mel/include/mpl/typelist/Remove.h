#pragma once
#include <mpl/typelist/TypeList.h>


namespace mel
{
	namespace mpl
	{
		namespace typelist
		{
			//remove element in pos
			template <class TList,int index> struct Remove;
			template <int index> struct Remove<NullType,index> //no estoy muy seguro de esta especializacion
			{
				typedef NullType Result;
			};
			template <class H,class T,int index> struct Remove< TypeList<H,T>,index >
			{
				typedef TypeList< H, typename Remove< T,index-1>::Result > Result;
			};
			template <class H,class T> struct Remove< TypeList<H,T>,0 >
			{
				typedef T Result;
			};


		}
	}
}