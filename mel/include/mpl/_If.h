#pragma once

/**
* @namespace mpl
* @brief metaprogramming library
* @remarks This library dates back to the early 2000s, when no C++11 existed, and so all its metaprogramming stuff. Most of this library can be replaced by equivalent
* C++11 functionalities, but because it's heavily used in the code, it's not removed. Slowly, its use is being replaced with the C++ facilities with the hope to be
removed some day
*/
namespace mel
{
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
}