#pragma once
#include <mpl/typelist/TypeList.h>

namespace mel
{
	namespace mpl
	{
		namespace typelist
		{
			//busca el primer elemento que cumpla condicion. La condicion ser� del estilo Condition<T>::result
			//devuelve -1 si no se encuentra
			namespace _private
			{
				template <class TList, template <class> class Condition,int currentPos> struct TListFindPos;
				template <class TList,template <class> class Condition,int currentPos,bool> struct NextPos;
				template <class TList,template <class> class Condition,int currentPos> struct NextPos<TList,Condition,currentPos,true>
				{
					enum {result = currentPos};
				};
				template <class H,class T,template <class> class Condition,int currentPos> struct NextPos<TypeList<H,T>,Condition,currentPos,false>
				{
					enum {result = TListFindPos< T, Condition,currentPos+1>::result };
				};

				template <class H, class T, template <class> class Condition,int currentPos> struct TListFindPos< TypeList<H,T>, Condition,currentPos >
				{
					enum { result = NextPos<TypeList<H,T>,Condition,currentPos,Condition<H>::result>::result };
				};
				template <template <class> class Condition,int currentPos> struct TListFindPos< NullType, Condition,currentPos >
				{
					enum { result = -1 };
				};

			}
			template <class TList, template <class> class Condition> struct Find
			{
				enum { result = _private::TListFindPos< TList,Condition,0 >::result };
			};


			//search. Find first ocurrence of a type
			template <class TList,class type> struct FindFirst
			{
				template <class U> struct _condition
				{
					enum {result =  mel::mpl::isSame<type,U>::result};
				};
				enum { result = TListFind< TList, _condition>::result };
			};


		}
	}
}