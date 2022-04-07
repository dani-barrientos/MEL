#pragma once
#include <mpl/typelist/TypeList.h>
#include <mpl/_If.h>
#include <mpl/typelist/Length.h>
namespace mel
{
	namespace mpl
	{
		namespace typelist
		{
			//element access. Usage Element< ..type list.., n >::Result
			//TODO hacer version que no tenga valor por defecto
			template <class TList,int pos,bool checkbounds,class Default = EmptyType> struct Element;
			//TODO apa�o para poder salirme de limites.No esta bien la comprobacion de limites
			template <int pos,bool checkbounds,class Default> struct Element< NullType, pos,checkbounds,Default>
			{
				typedef Default Result;
			};
			template < class H, class T,int pos,bool checkbounds,class Default > struct Element< TypeList<H,T>,pos,checkbounds,Default >
			{
				enum { isGreater=Length< TypeList<H,T> >::result > pos};
				typedef typename _if< isGreater,typename Element< T,pos-1,checkbounds,Default>::Result,Default>::Result Result;
				//typedef typename _if< isGreater,typename Element< T,pos-1,checkbounds,Default>::Result,Default>::Result Result
			};

			/* template < class H, class T,int pos,bool checkbounds,class Default > struct Element< TypeList<H,T>,pos,checkbounds,Default >
			{
			enum { isGreater=(Length< TypeList<H,T> >::result > pos || !checkbounds)};
			//enum { isGreater=true};
			typedef typename _if< isGreater,typename Element< T,!checkbounds?pos-1:0,checkbounds,Default>::Result,Default>::Result Result;
			//typedef typename _if< isGreater,typename Element< T,pos-1,checkbounds,Default>::Result,Default>::Result Result;

			};*/
			template < class H, class T,bool checkbounds,class Default> struct Element< TypeList<H,T>,0, checkbounds,Default >
			{
				typedef H Result;
			};


		}
	}
}