/**
* some useful macros for multithreading problems
*/
#pragma once
#include <preprocessor/params.h>
#include <mpl/LinkArgs.h>
using mel::mpl::linkFunctor;


#include <mpl/ReturnAdaptor.h>
#include <core/Future.h>
using mel::core::Future;

#include <core/Runnable.h>
using mel::core::Runnable;
#include <core/UnknownException.h>
using mel::core::UnknownException;

namespace mel
{
	namespace core
	{
	#define GET_ARGS( args ) args
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





		/**
		* define a method synchronized with a given runnable.
		* @params[in] TRet. Return type
		* @params[in] function_name. funtion to create
		* @params[in] runnable: Runnable (pointer) in which function is executed
		* @return function return. Throws Exception* if some error ocurred.If The error causing
		* exception is a known exception, then this is thrown else it returns an UnknownException
		
		*/
	#define SYNCHRONIZED( TRet, function_name,args,runnable) \
		TRet function_name##_sync args ; \
		TRet function_name( MAKE_PARAMS args  ){ \
			Future<TRet> result = runnable->execute<TRet>(  CREATE_LINKER( TRet,function_name##_sync,args ) ); \
			result.wait();\
			if ( !result.getValid() ){\
			if ( result.getError()->error == Runnable::ERRORCODE_EXCEPTION ){\
					Runnable::ExecuteErrorInfo* ei = (Runnable::ExecuteErrorInfo*)result.getError();\
					if ( ei->isPointer ) \
					throw ei->exc;\
					else{ ei->exc->throwSame();return result.getValue();} /*this is only to avoid compiler warning. It will nevere be executed */ \
				}else \
					throw ::mel::core::UnknownException();\
			}else return result.getValue();}
	//@todo lo que viene es �apa por que lo necesito ya,para funciones static. Lo pensar� mejor m�s adelante
	#define SYNCHRONIZED_STATIC( TRet, function_name,args,runnable) \
		static TRet function_name##_sync args ; \
		static TRet function_name( MAKE_PARAMS args  ){ \
		Future<TRet> result = runnable->execute<TRet>(  CREATE_LINKER( TRet,function_name##_sync,args ) ); \
		result.wait();\
		if ( !result.getValid() ){\
		if ( result.getError()->error == Runnable::ERRORCODE_EXCEPTION ){\
		Runnable::ExecuteErrorInfo* ei = (Runnable::ExecuteErrorInfo*)result.getError();\
				if ( ei->isPointer ) \
					throw ei->exc; \
				else{ ei->exc->throwSame();return result.getValue();} /*this is only to avoid compiler warning. It will nevere be executed */ \
			}else \
			throw ::mel::core::UnknownException();\
		}else return result.getValue();}

		/**
		* similar to SYNCHRONIZED_METHOD but doesn't wait for execution, only post it. So return value is returned by default
		* this posted method won't return any value, so internally is void and it will return a default value before being posted
		* @warning this way OF definig functions is dangerous. Function is posted, so arguments maybe are not yet valid
		* @todo no estoy nada seguro de esto
		*/
	/*#define POSTED( TRet, function_name,args,runnable,defaultvalue) \
		void function_name##_sync args ; \
		TRet function_name( MAKE_PARAMS args  ){ \
		runnable->post( RUNNABLE_CREATETASK(::mel::mpl::returnAdaptor<void>( CREATE_LINKER( void,function_name##_sync,args ),true ) ); \
		return defaultvalue;}
	*/
	#define POSTED( TRet, function_name,args,runnable,defaultvalue) \
		void function_name##_sync args ; \
		TRet function_name( MAKE_PARAMS args  ){ \
		runnable->post( RUNNABLE_CREATETASK((::mel::mpl::returnAdaptor< void>( CREATE_LINKER( void,function_name##_sync,args ),true ) )) ); \
		return defaultvalue;}

	#define POSTED_STATIC( TRet, function_name,args,runnable,defaultvalue) \
		static void function_name##_sync args ; \
		static TRet function_name( MAKE_PARAMS args  ){ \
		runnable->post( RUNNABLE_CREATETASK((::mel::mpl::returnAdaptor< void>( CREATE_LINKER( void,function_name##_sync,args ),true ) )) ); \
		return defaultvalue;}	
	//same for class member functions

	//SACAR LA CLASE DEL THIS! Por ahora cutre obligando a tener definido MyType de la forma: typedef Clase MyType
	//define a method synchronized with a Runnable. Wait for execution completed
	#define SYNCHRONIZED_METHOD( TRet, function_name,args,runnable) \
		TRet function_name##_sync args ; \
		TRet function_name( MAKE_PARAMS args  ){ \
		Future<TRet> result = runnable->execute<TRet>(  CREATE_LINKER( TRet,makeMemberEncapsulate( &MyType::function_name##_sync,this),args ) ); \
		result.wait();\
		if ( !result.getValid() ){\
		if ( result.getError()->error == Runnable::ERRORCODE_EXCEPTION ){\
			Runnable::ExecuteErrorInfo* ei = (Runnable::ExecuteErrorInfo*)result.getError();\
			if ( ei->isPointer ) \
				throw ei->exc; \
			else{ ei->exc->throwSame();return result.getValue();} /*this is only to avoid compiler warning. It will nevere be executed */ \
		}else \
			throw ::mel::core::UnknownException();\
		}else return result.getValue();}

	#define SYNCHRONIZED_METHOD_OVERRIDE( TRet, function_name,args,runnable) \
		TRet function_name##_sync args ; \
		TRet function_name( MAKE_PARAMS args  ) override { \
		Future<TRet> result = runnable->execute<TRet>(  CREATE_LINKER( TRet,makeMemberEncapsulate( &MyType::function_name##_sync,this),args ) ); \
		result.wait();\
		if ( !result.getValid() ){\
		if ( result.getError()->error == Runnable::ERRORCODE_EXCEPTION ){\
			Runnable::ExecuteErrorInfo* ei = (Runnable::ExecuteErrorInfo*)result.getError();\
			if ( ei->isPointer ) \
				throw ei->exc; \
			else{ ei->exc->throwSame();return result.getValue();} /*this is only to avoid compiler warning. It will nevere be executed */ \
		}else \
			throw ::mel::core::UnknownException();\
		}else return result.getValue();}

	#define POSTED_METHOD( TRet, function_name,args,runnable,defaultvalue) \
		void function_name##_sync args ; \
		TRet function_name( MAKE_PARAMS args  ){ \
		runnable->post( RUNNABLE_CREATETASK( (::mel::mpl::returnAdaptor<void>( CREATE_LINKER( void, makeMemberEncapsulate(&MyType::function_name##_sync,this),args),true ) )) ); \
		return defaultvalue;}
	}
}