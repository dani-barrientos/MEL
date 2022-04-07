#pragma once

namespace mpl
{
    namespace typelist
    {
        /**
        * Replace an elment (given its index) of a TypeList with a new type
        **/
        template <class TList,int pos,class NewType> struct Replace;
        template <int index,class NewType> struct Replace<NullType,index,NewType> //no estoy muy seguro de esta especializacion
		{
			typedef NullType Result;
		};
		template <class H,class T,int index,class NewType> struct Replace< TypeList<H,T>,index,NewType >
		{
			typedef TypeList< H, typename Replace< T,index-1,NewType>::Result > Result;
		};
		template <class H,class T,class NewType> struct Replace< TypeList<H,T>,0,NewType >
		{
			typedef TypeList<NewType,T> Result;
		};
    }
}
