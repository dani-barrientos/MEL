#pragma once

namespace mel
{
	namespace mpl
	{
		namespace typelist
		{
			/**
			* Replace all elements in a TypeList with de result of apply de template W. For example:
			* given \code typelist TL (int,float) and template <class T> struct MiClass{ T value; }
			* ReplateAll< TL,MiClass >::Result \endcode return a new TypeList where each element is the previous element wrapped by MiClass
			*/
			template <class TList,template <class> class W> struct ReplaceAll;
			///@cond
			template <class H,class T,template <class> class W> struct ReplaceAll< TypeList<H,T>, W>
			{
				typedef TypeList< W<H>, typename ReplaceAll<T,W>::Result > Result;
			};
			template <template <class> class W> struct ReplaceAll< NullType,W>
			{
				typedef NullType Result;
			};
			//@endcond
		}
	}
}