#pragma once
#include <mpl/TypeTraits.h>

namespace mel
{
    namespace mpl
    {

        template <class T> struct Ref
        {
        // Ref( const Ref& r) : mObj( r.mObj ){}
        Ref( T& obj):mObj(obj){}
        inline  operator T&() const
        {
            return mObj;
        }
        inline Ref& operator =( typename TypeTraits<T>::ParameterType ob2 )
        {
            mObj = ob2;
            return *this;
        }
        bool operator==( const Ref& ob2 ) const
        {
            return &mObj == &ob2.mObj;
        }
        private:
        T& mObj;
        };
        template <class T> Ref<T> createRef( T& obj)
        {
            return Ref<T>( obj );
        }
    }
}