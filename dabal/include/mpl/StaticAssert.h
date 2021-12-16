#pragma once

namespace mpl
{

    /**
    * assert equivalent but in compiletime
    * usage:
    *   MPL_STATIC_ASSERT( condition,errormsg)
    * Error message is not a quoted string
    *  Example:
    * MPL_STATIC_ASSERT( mpl::isSame<int,float>::result,NotSameType)
    **/

   template <bool b>
   struct StaticAssert
   {
        StaticAssert(...){};
        //!@note: stupid dummy method to avoid unused entity warnings from clang
        inline void dummy() {}
   };
   template <>
   struct StaticAssert<false>
   {
       //!@note: stupid dummy method to avoid unused entity warnings from clang
       inline void dummy() {}
   };
#ifndef NDEBUG
   #define MPL_STATIC_ASSERT( value, msg ) \
   do {                                 \
    struct ERROR_##msg{};                \
    typedef mpl::StaticAssert<value> AuxT; \
    AuxT aux = AuxT( ERROR_##msg()); \
    aux.dummy(); \
   } while(0)
#else
    #define MPL_STATIC_ASSERT( value, msg ) \
    do{ } while( 0 )
#endif
}
