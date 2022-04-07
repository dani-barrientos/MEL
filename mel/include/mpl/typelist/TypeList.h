#pragma once
#include <preprocessor/params.h>
#include <mpl/BasicTypes.h>
using mel::mpl::NullType;
#include <mpl/IsSame.h>
/*
* @todo sacar los algoritmos a ficheros aparte
*/
namespace mpl
{
	namespace typelist
	{
		template <class H,class T>
		struct TypeList
		{
			typedef H Head;
			typedef T Tail;
		};
#ifdef _MSC_VER
		//macros for easy definition
#define TYPELIST(...) TYPELIST_IMPL( (MACRO_NUM_ARGS( __VA_ARGS__ ),__VA_ARGS__) )
#define TYPELIST_IMPL( a ) TYPELIST_N a
#define TYPELIST_N( N, ... ) TYPELIST_(N,(__VA_ARGS__) )
#define TYPELIST_(N,args) TYPELIST__( N, args )
#define TYPELIST__(N,args) _TYPELIST_ ## N args
#else
		//macros for easy definition
#define TYPELIST(...) TYPELIST_N( MACRO_NUM_ARGS( __VA_ARGS__ ),__VA_ARGS__ )
#define TYPELIST_N( N, ... ) TYPELIST_(N,__VA_ARGS__ )
#define TYPELIST_(N,...) _TYPELIST_ ## N( __VA_ARGS__ )
#endif
#define _TYPELIST_0( ) NullType
#define _TYPELIST_1( T1 ) TypeList<T1,mpl::NullType>
#define _TYPELIST_2( T1,T2 ) TypeList< T1,_TYPELIST_1(T2) >
#define _TYPELIST_3( T1,T2,T3 ) TypeList< T1,_TYPELIST_2(T2,T3) >
#define _TYPELIST_4( T1,T2,T3,T4 ) TypeList< T1,_TYPELIST_3(T2,T3,T4) >
#define _TYPELIST_5( T1,T2,T3,T4,T5 ) TypeList< T1,_TYPELIST_4(T2,T3,T4,T5) >
#define _TYPELIST_6( T1,T2,T3,T4,T5,T6 ) TypeList< T1,_TYPELIST_5(T2,T3,T4,T5,T6) >
#define _TYPELIST_7( T1,T2,T3,T4,T5,T6,T7 ) TypeList< T1,_TYPELIST_6(T2,T3,T4,T5,T6,T7) >
#define _TYPELIST_8( T1,T2,T3,T4,T5,T6,T7,T8 ) TypeList< T1,_TYPELIST_7(T2,T3,T4,T5,T6,T7,T8) >
#define _TYPELIST_9( T1,T2,T3,T4,T5,T6,T7,T8,T9 ) TypeList< T1,_TYPELIST_8(T2,T3,T4,T5,T6,T7,T8,T9) >
#define _TYPELIST_10( T1,T2,T3,T4,T5,T6,T7,T8,T9,T10 ) TypeList< T1,_TYPELIST_9(T2,T3,T4,T5,T6,T7,T8,T9,T10) >
#define _TYPELIST_11( T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11 ) TypeList< T1,_TYPELIST_10(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11) >
#define _TYPELIST_12( T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12 ) TypeList< T1,_TYPELIST_11(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12) >
#define _TYPELIST_13( T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13 ) TypeList< T1,_TYPELIST_12(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13) >
#define _TYPELIST_14( T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14 ) TypeList< T1,_TYPELIST_13(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14) >
#define _TYPELIST_15( T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15 ) TypeList< T1,_TYPELIST_14(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15) >
#define _TYPELIST_16( T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16 ) TypeList< T1,_TYPELIST_15(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16) >
#define _TYPELIST_17( T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17 ) TypeList< T1,_TYPELIST_16(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17) >
#define _TYPELIST_18( T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18 ) TypeList< T1,_TYPELIST_17(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18) >
#define _TYPELIST_19( T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19 ) TypeList< T1,_TYPELIST_18(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19) >
#define _TYPELIST_20( T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,T20 ) TypeList< T1,_TYPELIST_19(T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,T20) >


		//!Create TypeList of size "size" with type T
		template <int size,class T> struct Create
		{
			typedef TypeList<T,typename Create< size-1,T>::Result> Result;
		};
		template <class T> struct Create<1,T>
		{
			typedef TYPELIST(T) Result;
		};

	}
}


