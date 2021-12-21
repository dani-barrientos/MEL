#pragma once

/**
* @namespace mpl
* @brief metaprogramming library
*/
namespace mpl
{
	template <bool Cond,class T,class U>
	struct _if
	{
		typedef T Result;
	};
	template <class T,class U>
	struct _if<false,T,U>
	{
		typedef U Result;
	};
	template <bool Cond,int t,int u> struct _ifInt
	{
	    enum {result = t};
	};
	template <int t,int u> struct _ifInt<false,t,u>
	{
	    enum {result = u};
	};

}
