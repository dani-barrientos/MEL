#pragma once
#include <mpl/typelist/TypeList.h>

namespace mpl
{
	namespace typelist
	{

		//append element
		template <class TList,class New> struct Append;

		template <class New> struct Append<NullType,New>
		{
			typedef TYPELIST(New) Result;
		};
		template <class H,class T,class New> struct Append< TypeList<H,T>,New >
		{
			typedef TypeList< H, typename Append<T,New>::Result > Result;
		};

	}
}