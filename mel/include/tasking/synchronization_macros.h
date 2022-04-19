#pragma once
/**
* @file 
* @brief Some useful macros for function synchronization
*/
///@cond HIDDEN_SYMBOLS
#include <preprocessor/params.h>
#include <mpl/LinkArgs.h>
using mel::mpl::linkFunctor;


#include <mpl/ReturnAdaptor.h>
#include <tasking/utilities.h>
#include <tasking/Runnable.h>
using mel::tasking::Runnable;
namespace mel
{
	namespace core
	{
	#define GET_ARG_1( _1,... ) _1
	#define GET_ARG_2( _1,_2,... ) _2
	#define GET_ARG_3( _1,_2,_3,... ) _3
	#define GET_ARG_4( _1,_2,_3,_4,... ) _4
	#define GET_ARG_5( _1,_2,_3,_4,_5,... ) _5
	#define GET_ARG_6( _1,_2,_3,_4,_5,_6,... ) _6
	#define GET_ARG_7( _1,_2,_3,_4,_5,_6,_7,... ) _7
	#define GET_ARG_8( _1,_2,_3,_4,_5,_6,_7,_8,... ) _8
	#define GET_ARG_9( _1,_2,_3,_4,_5,_6,_7,_8,_9,... ) _9
	#define GET_ARG_10( _1,_2,_3,_4,_5,_6,_7,_8,_9,_10,... ) _10


	#define CREATE_LINKER( TRet,function_name,args ) CREATE_LINKER_IMPL( MACRO_NUM_ARGS args ,TRet,function_name,args )
	#define CREATE_LINKER_IMPL( N,TRet,function_name,args ) CREATE_LINKER_IMPL_( N ,TRet,function_name,args )
	#define CREATE_LINKER_IMPL_(N,TRet,function_name,args) CREATE_LINKER_N( N ,TRet,function_name,args )
	//#define CREATE_LINKER_N( N,TRet,function_name,args) CREATE_LINKER_ ## N( TRet,function_name, args  )
	#define CREATE_LINKER_N( N,TRet,function_name,args) CREATE_LINKER_N_(N, TRet,function_name, args  )
	#define CREATE_LINKER_N_( N,TRet,function_name,args) CREATE_LINKER_ ## N( TRet,function_name, args  )
	#define CREATE_LINKER_0(TRet,function_name,args) function_name
	#define CREATE_LINKER_1(TRet,function_name,args) \
		linkFunctor<TRet,TYPELIST(),GET_ARG_1 args >( function_name,_1 )
		
	#define CREATE_LINKER_2(TRet,function_name,args ) \
		linkFunctor<TRet, TYPELIST(),GET_ARG_1 args,GET_ARG_2 args >( function_name,_1,_2 )  
	#define CREATE_LINKER_3(TRet,function_name,args ) \
		linkFunctor<TRet,TYPELIST(),GET_ARG_1 args,GET_ARG_2 args,GET_ARG_3 args >( function_name,_1,_2,_3 )
	#define CREATE_LINKER_4(TRet,function_name,args ) \
		linkFunctor<TRet,TYPELIST(),GET_ARG_1 args,GET_ARG_2 args,GET_ARG_3 args,GET_ARG_4 args >( function_name,_1,_2,_3,_4 )
	#define CREATE_LINKER_5(TRet,function_name,args ) \
		linkFunctor<TRet,TYPELIST(),GET_ARG_1 args,GET_ARG_2 args,GET_ARG_3 args,GET_ARG_4 args,GET_ARG_5 args >( function_name,_1,_2,_3,_4,_5 )

	#define CREATE_LINKER_6(TRet,function_name,args ) \
		linkFunctor<TRet,TYPELIST(),GET_ARG_1 args,GET_ARG_2 args,GET_ARG_3 args,GET_ARG_4 args,GET_ARG_5 args,GET_ARG_6 args >( function_name,_1,_2,_3,_4,_5,_6 )
	#define CREATE_LINKER_7(TRet,function_name,args ) \
		linkFunctor<TRet,TYPELIST(),GET_ARG_1 args,GET_ARG_2 args,GET_ARG_3 args,GET_ARG_4 args,GET_ARG_5 args,GET_ARG_6 args,GET_ARG_7 args>( function_name,_1,_2,_3,_4,_5,_6,_7 )

/*
old mpl functions/classes haven't any noexcept considerations (not existed at all in C++98), so changed in favor of a labmda based mechanism
	#define SYNCHRONIZED_STATIC( TRet, function_name,args,runnable,qualifiers) \
		static TRet function_name##_sync args qualifiers; \
		static Future<TRet> function_name( MAKE_PARAMS args  ) qualifiers{ \
		return runnable->execute<TRet>(  CREATE_LINKER( TRet,function_name##_sync,args ) );}
	#define SYNCHRONIZED_METHOD( TRet, function_name,args,runnable,qualifiers) \
		TRet function_name##_sync args qualifiers; \
		Future<TRet> function_name( MAKE_PARAMS args  ) qualifiers{ \
		return runnable->execute<TRet>(  CREATE_LINKER( TRet,makeMemberEncapsulate( &std::remove_reference_t<decltype(*this)>::function_name##_sync,this),args ) );}
*/
///@endcond
	/**
	* @brief Declare a static function synchronized with a given runnable.
	* @details The code need to implement that *no sync* funcion (so, a normal funcion) which its call is managed by this macro. For example, if 
	* we declare a synchronized function as \verbatim SYNCHRONIZED_STATIC( f1,string,(int),noexcept,sRunnable ) \endverbatim, we have to implement the neccesary code for `f1_sync`, as:
	* \code
	* 
	static string f1_sync(int v) noexcept
	{
		return std::to_string(v);		
	}
	* \endcode
	* @param[in] TRet. Return type
	* @param[in] function_name. funtion to create
	* @param[in] args parameter types (without parameter names), in the form (T1,T2,...) [0..10] number of parameters
	* @param[in] qualifiers: extra function qualifiers. Can be left empty
	* @param[in] runnable: Runnable (pointer) in which function is executed
	* @return \ref mel::core::Future<TRet>
	*/
	#define SYNCHRONIZED_STATIC( function_name,TRet,args,qualifiers,runnable) \
		static TRet function_name##_sync args qualifiers; \
		static Future<TRet> function_name( MAKE_PARAMS args  ) qualifiers{ \
		return runnable->execute<TRet>( [CALL_PARAMS args]() qualifiers{ \
            return function_name##_sync(CALL_PARAMS args);\
        });}
	/**	
	* @brief Declare a method synchronized with a given runnable.
	* @details The code need to implement that *no sync* funcion (so, a normal funcion) which its call is managed by this macro. For example, if 
	* we declare a synchronized function as \verbatim SYNCHRONIZED_METHOD( f1,string,(int,float),,sRunnable ) \endverbatim, we have to implement the neccesary code for `f1_sync`, as:
	* \code
	* 
	static string f1_sync(int a,float b )
	{
		return std::to_string(a) + std::to:string(b);		
	}
	* \endcode
	* @param[in] function_name. funtion to create
	* @param[in] TRet. Return type
	* @param[in] args parameter types (without parameter names), in the form (T1,T2,...) [0..10] number of parameters
	* @param[in] qualifiers: extra function qualifiers. Can be left empty
	* @param[in] runnable: Runnable (pointer) in which function is executed
	* @return \ref mel::core::Future<TRet>
	*/
	#define SYNCHRONIZED_METHOD( function_name,TRet,args,qualifiers,runnable) \
		TRet function_name##_sync args qualifiers; \
		Future<TRet> function_name( MAKE_PARAMS args  ) qualifiers{ \
        return runnable->execute<TRet>( [this, CALL_PARAMS args]() qualifiers{ \
            return function_name##_sync(CALL_PARAMS args);\
        });}
///@cond HIDDEN_SYMBOLS		
	}	
}
///@endcond