#pragma once
#include <mpl/TypeTraits.h>
#include <mpl/equal.h>
namespace mel
{
    namespace mpl
    {
        /**
        * makes an object to behave as a pointer
        * Example: you can use a object as argument for a MEncapsulate.
        *   Suppose you have the code:
        *   struct Test
            {
            void funcion(){};
            }

            Test test;
            makeMemberEncapsulate( &Test::funcion, &test );
            ... save the created object somewhere or post to a thread,...
            previous code is incorrect because test is local, so &test will point
            to a local variable that is destroyed becasue the result of makeMemberEncapsulate is used
            outside this scope.
            The solution is:
            makeMemberEncapsulate( &Test::funcion, asPtr( test ) )
            This way asPtr makes MEncapsulate believe that test is a pointer holding an internal copy
            of test

        *
        */
        template <class T> class AsPtr
        {
            private:
            T mObject;
            public:
            AsPtr( const T& object ):mObject( object ){}
            AsPtr( const AsPtr& object ):mObject( object.mObject ){}
            inline const T* operator->() const
            {
                return &mObject;
            }
            inline T* operator->()
            {
                return &mObject;
            }
            inline operator T*()
            {
                return &mObject;
            }
            inline const T& operator*() const
            {
                return mObject;
            }
            inline T& operator*()
            {
                return mObject;
            }
            inline bool operator == ( const AsPtr& ob2 ) const;
        };
        // helper function for argument deduction
        template <class T> AsPtr<T> asPtr( T object )
        {
            return AsPtr<T>( object );
        }
        template <class T>
        bool AsPtr<T>::operator == ( const AsPtr& ob2 ) const
        {
            //return mObject == ob2.mObject;�como saber si tiene operator==
            //return Compare<::mel::mpl::ComparableTraits<T>::Result,T >::compare( mObject,ob2.mObject );
            //return Compare<::mel::mpl::ComparableTraits<T>::Result >::compare( mObject,ob2.mObject );
            return mel::mpl::equal<true>( mObject, ob2.mObject );
        }
    }
}