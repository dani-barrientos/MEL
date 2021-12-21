#pragma once

#include <mpl/typelist/TypeList.h>
#include <mpl/typelist/Element.h>

/**
* generate hierarchy from typelist 
*/
namespace mpl
{
	using mpl::typelist::TypeList;
	using mpl::typelist::Element;
	namespace _private
	{
		template <class TList/*,template <class> class Holder*/>
		class CreateWideInheritance_Base;
		template < class H,class T/*,template <class > class Holder*/> class CreateWideInheritance_Base< TypeList<H,T>/*, Holder*/ > : 
			public CreateWideInheritance_Base<H/*,Holder*/>,
			public CreateWideInheritance_Base<T/*,Holder*/>
		{
		};
		template < class SimpleType/*,template <class> class Holder */> class CreateWideInheritance_Base: public SimpleType
		{
		};
		template < /*template <class> class Holder */> class CreateWideInheritance_Base< NullType/*, Holder*/ >
		{
		};
	}
	template <class TList/*, template <class> class Holder*/> class CreateWideInheritance : public _private::CreateWideInheritance_Base<TList/*,Holder*/>
	{
	public:
		template<int n> typename Element<TList,n,true>::Result& get()
		{
			return static_cast<typename Element<TList,n,true>::Result&>(*this);
		}

	

	};



}