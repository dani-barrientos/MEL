#pragma once
/**
* common useful types used in mpl
*/
namespace mpl
{
	class Small{ char dummy[1];};
	class Big { char dummy[2]; };
	struct AnyType
	{
		template <class T> AnyType( const T& )  {};
		//Small operator==( const AnyType& ) const;
		
		//friend Small operator==(const AnyType&, const AnyType&);
		//friend Small operator==(const AnyType&, const AnyType&);
		//template<class T> friend Small operator==(const AnyType&, const T&);
		//template<class T> friend Small operator==(const T&, const AnyType&);
//		friend template<class T> Small operator==(const AnyType&, const T&);
	};

}
//::mpl::Small operator==(const ::mpl::AnyType&, const ::mpl::AnyType&);