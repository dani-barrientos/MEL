#pragma once
#include <mpl/typelist/TypeList.h>
#include <mpl/typelist/Element.h>

namespace mpl
{
    namespace typelist
    {
        namespace _private
        {

            template <class TList,int index,class F> struct _ForEach;
            template <class H,class T,int index, class F> struct _ForEach< TypeList<H,T>, index,F>
            {
                static void doIt( F& functor)
                {
					#ifdef _WINDOWS
						functor.operator()<H>();
					#else
						//valid in a true C++ conformant compiler
						functor.template operator()<H>();
					#endif

                    _ForEach< T,index+1,F>::doIt( functor );
                }
            };
            template <int index,class F> struct _ForEach< NullType, index,F>
            {
                static void doIt( F&){}
            };

        }

		/**
		* do some operation on each type in a list.
		* Given Functor is a function object with a template operator() where template argument is each type in list. Example:
		* @code
		*	struct MyFunctor
		*	{
		*		template <class T> void operator()()
		*		{
		*			... do some operation on type T ...
		*		}
		*	}
		* @endcode
		**/
        template <class TList> struct ForEach
        {
            template < class F> static  inline void doIt( F functor )
            {
                _private::_ForEach< TList, 0 ,F >::doIt( functor);
            }
        };
    }
}
