#pragma once
/**
* macros para preprocesado útiles. Todavía muy simplón, todo metido aquí. Más adelante estará separadas las cosas por ficheros
*/
namespace preprocessor
{
/**
* número de argumentos en macro
*/
/*
version perfectamente funcional en GCC
#define MACRO_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 5,4,3,2,1)
#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,N,...) N
*/
//TENGO QUE HACER PATRAÑAS PARA QUE FUNCIONE EN VISUAL
#define _DABAL_PREP_ARGS(_0, _1, ...) _1
#define TWO_ARGS(...) TWO_ARGS_IMPL( (__VA_ARGS__,1) )
#define TWO_ARGS_IMPL( args ) _DABAL_PREP_ARGS args
#define TESTCASE_(...) 1,0
#define ONEORZERO(...) TWO_ARGS( TESTCASE_ ##__VA_ARGS__() )

#define MACRO_NUM_ARGS(...) MACRO_NUM_ARGS_IMPL_( (__VA_ARGS__, 10,9,8,7,6,5,4,3,2,1,0) )
#define MACRO_NUM_ARGS_IMPL_( a ) MACRO_NUM_ARGS_IMPL a
#define MACRO_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) MACRO_NUM_ARGS_IMPL_ ## N( _1 )
#define MACRO_NUM_ARGS_IMPL_1(...) ONEORZERO( __VA_ARGS__ )
#define MACRO_NUM_ARGS_IMPL_2(...) 2
#define MACRO_NUM_ARGS_IMPL_3(...) 3
#define MACRO_NUM_ARGS_IMPL_4(...) 4
#define MACRO_NUM_ARGS_IMPL_5(...) 5
#define MACRO_NUM_ARGS_IMPL_6(...) 6
#define MACRO_NUM_ARGS_IMPL_7(...) 7
#define MACRO_NUM_ARGS_IMPL_8(...) 8
#define MACRO_NUM_ARGS_IMPL_9(...) 9
#define MACRO_NUM_ARGS_IMPL_10(...) 10

/*
#define MACRO_NUM_ARGS(...) MACRO_NUM_ARGS_IMPL_( (__VA_ARGS__, 10,9,8,7,6,5,4,3,2,1,0) )
#define MACRO_NUM_ARGS_IMPL_( a ) MACRO_NUM_ARGS_IMPL a 
#define MACRO_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) N
*/
/**
* construcción de argumentos a partir de una lista tal que: (T1,T2,T3,...). Los Ti son tipos
* Construye una lista de la forma:(T1 _1, T2 _2, T3 _3,..)  (hasta 10 parámetros, puede ser ampliado fácilmente
*/
#ifdef _MSC_VER
	
	//el visual(al menos el 2005) no procesa igual que el gcc las macros
	#define MAKE_PARAMS( ... ) MAKE_PARAMS_IMPL( (MACRO_NUM_ARGS( __VA_ARGS__ ),__VA_ARGS__)  )
	#define MAKE_PARAMS_IMPL( a ) MAKE_PARAMS_N a
	#define MAKE_PARAMS_N(N,...) MAKE_PARAMS_(N, (__VA_ARGS__) )
	//#define MAKE_PARAMS_(N,args) MAKE_PARAMS_ ## N args
	#define MAKE_PARAMS_(N,args) MAKE_PARAMS__(N, args)
	#define MAKE_PARAMS__(N,args) MAKE_PARAMS_ ## N args

	#define MAKE_PARAMS_0( ) 		
	#define MAKE_PARAMS_1( a ) a _1		
	#define MAKE_PARAMS_2( a,b ) MAKE_PARAMS_1( a ),b _2
	#define MAKE_PARAMS_3( a,b,c ) MAKE_PARAMS_2(a,b), c _3
	#define MAKE_PARAMS_4( a,b,c,d ) MAKE_PARAMS_3(a,b,c), d _4
	#define MAKE_PARAMS_5( a,b,c,d,e ) MAKE_PARAMS_4(a,b,c,d), e _5
	#define MAKE_PARAMS_6( a,b,c,d,e,f ) MAKE_PARAMS_5(a,b,c,d,e), f _6
	#define MAKE_PARAMS_7( a,b,c,d,e,f,g ) MAKE_PARAMS_6(a,b,c,d,e,f), g _7
	#define MAKE_PARAMS_8( a,b,c,d,e,f,g,h ) MAKE_PARAMS_7(a,b,c,d,e,f,g), h _8
	#define MAKE_PARAMS_9( a,b,c,d,e,f,g,h,i ) MAKE_PARAMS_8(a,b,c,d,e,f,g,h), i _9
	#define MAKE_PARAMS_10( a,b,c,d,e,f,g,h,i,j ) MAKE_PARAMS_9(a,b,c,d,e,f,g,h,i), j _10
#else
	 //para GCC. Estoy suponiendo que no hay más compiladores...
	#define MAKE_PARAMS( ... ) MAKE_PARAMS_N( MACRO_NUM_ARGS( __VA_ARGS__ ),__VA_ARGS__ )
	#define MAKE_PARAMS_N(N,...) MAKE_PARAMS_(N, __VA_ARGS__ )
	#define MAKE_PARAMS_(N,...) MAKE_PARAMS_ ## N( __VA_ARGS__ )
	#define MAKE_PARAMS_0( )
	#define MAKE_PARAMS_1( a ) a _1
	#define MAKE_PARAMS_2( a,b ) MAKE_PARAMS_1( a ),b _2
	#define MAKE_PARAMS_3( a,b,c ) MAKE_PARAMS_2(a,b), c _3
	#define MAKE_PARAMS_4( a,b,c,d ) MAKE_PARAMS_3(a,b,c), d _4
	#define MAKE_PARAMS_5( a,b,c,d,e ) MAKE_PARAMS_4(a,b,c,d), e _5
	#define MAKE_PARAMS_6( a,b,c,d,e,f ) MAKE_PARAMS_5(a,b,c,d,e), f _6
	#define MAKE_PARAMS_7( a,b,c,d,e,f,g ) MAKE_PARAMS_6(a,b,c,d,e,f), g _7
	#define MAKE_PARAMS_8( a,b,c,d,e,f,g,h ) MAKE_PARAMS_7(a,b,c,d,e,f,g), h _8
	#define MAKE_PARAMS_9( a,b,c,d,e,f,g,h,i ) MAKE_PARAMS_8(a,b,c,d,e,f,g,h), i _9
	#define MAKE_PARAMS_10( a,b,c,d,e,f,g,h,i,j ) MAKE_PARAMS_9(a,b,c,d,e,f,g,h,i), j _10
#endif
/**
* crea lista de argumentos de llamada de la forma (_1,_2,_3,...)
*/
#define CALL_PARAMS( ... ) CALL_PARAMS_N( MACRO_NUM_ARGS( __VA_ARGS__ ) )
#define CALL_PARAMS_N(N,...) CALL_PARAMS_( N )
#define CALL_PARAMS_(N,...) CALL_PARAMS_ ##N
#define CALL_PARAMS_1 _1
#define CALL_PARAMS_2 CALL_PARAMS_1,_2
#define CALL_PARAMS_3 CALL_PARAMS_2,_3
#define CALL_PARAMS_4 CALL_PARAMS_3,_4
#define CALL_PARAMS_5 CALL_PARAMS_4,_5
#define CALL_PARAMS_6 CALL_PARAMS_5,_6
#define CALL_PARAMS_7 CALL_PARAMS_6,_7
#define CALL_PARAMS_8 CALL_PARAMS_7,_8
#define CALL_PARAMS_9 CALL_PARAMS_8,_9
#define CALL_PARAMS_10 CALL_PARAMS_9,_10
}