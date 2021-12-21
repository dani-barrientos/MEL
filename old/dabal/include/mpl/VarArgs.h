

#define VARIABLE_MAX_ARGS 7
#define coma ,

#define VARIABLE_NUM_ARGS 7

#undef VARIABLE_ARGS
#undef VARIABLE_ARGS_NODEFAULT
#undef VARIABLE_ARGS_DECL
#undef VARIABLE_ARGS_INVERTED
#undef VARIABLE_ARGS_IMPL
#undef VARIABLE_ARGS_USE

#define VARIABLE_ARGS class Arg1 = void coma class Arg2 = void coma class Arg3 = void coma class Arg4 = void coma class Arg5 = void coma class Arg6 = void coma class Arg7 = void
#define VARIABLE_ARGS_NODEFAULT class Arg1 coma class Arg2 coma class Arg3 coma class Arg4 coma class Arg5 coma class Arg6 coma class Arg7
#define VARIABLE_ARGS_INVERTED class Arg7 coma class Arg6 coma class Arg5 coma class Arg4 coma class Arg3 coma class Arg2 coma class Arg1
#define VARIABLE_ARGS_DECL Arg1 coma Arg2 coma Arg3  coma Arg4 coma Arg5 coma Arg6 coma Arg7
#define VARIABLE_ARGS_IMPL Arg1 arg1 coma Arg2 arg2 coma Arg3 arg3 coma Arg4 arg4 coma Arg5 arg5 coma Arg6 arg6 coma Arg7 arg7
#define VARIABLE_ARGS_USE arg1 coma arg2 coma arg3 coma arg4 coma arg5 coma arg6 coma arg7


#include INCLUDE_PATH

#undef VARIABLE_NUM_ARGS
#define VARIABLE_NUM_ARGS 6

#undef VARIABLE_ARGS
#undef VARIABLE_ARGS_NODEFAULT
#undef VARIABLE_ARGS_DECL
#undef VARIABLE_ARGS_INVERTED
#undef VARIABLE_ARGS_IMPL
#undef VARIABLE_ARGS_USE

#define VARIABLE_ARGS class Arg1 coma class Arg2 coma class Arg3 coma class Arg4 coma class Arg5 coma class Arg6
//#define VARIABLE_ARGS_NODEFAULT class Arg1 coma class Arg2 coma class Arg3 coma class Arg4 coma class Arg5
#define VARIABLE_ARGS_INVERTED class Arg6 coma class Arg5 coma class Arg4 coma class Arg3 coma class Arg2 coma class Arg1
#define VARIABLE_ARGS_DECL Arg1 coma Arg2 coma Arg3  coma Arg4 coma Arg5 coma Arg6
#define VARIABLE_ARGS_IMPL Arg1 arg1 coma Arg2 arg2 coma Arg3 arg3 coma Arg4 arg4 coma Arg5 arg5 coma Arg6 arg6
#define VARIABLE_ARGS_USE arg1 coma arg2 coma arg3 coma arg4 coma arg5 coma arg6
/*#define VARIABLE_NUM_ARGS 6

#undef VARIABLE_ARGS
#undef VARIABLE_ARGS_NODEFAULT
#undef VARIABLE_ARGS_DECL
#undef VARIABLE_ARGS_INVERTED
#undef VARIABLE_ARGS_IMPL
#undef VARIABLE_ARGS_USE

#define VARIABLE_ARGS class Arg1 = void coma class Arg2 = void coma class Arg3 = void coma class Arg4 = void coma class Arg5 = void coma class Arg6 = void
#define VARIABLE_ARGS_NODEFAULT class Arg1 coma class Arg2 coma class Arg3 coma class Arg4 coma class Arg5 coma class Arg6
#define VARIABLE_ARGS_INVERTED class Arg6 coma class Arg5 coma class Arg4 coma class Arg3 coma class Arg2 coma class Arg1
#define VARIABLE_ARGS_DECL Arg1 coma Arg2 coma Arg3  coma Arg4 coma Arg5 coma Arg6
#define VARIABLE_ARGS_IMPL Arg1 arg1 coma Arg2 arg2 coma Arg3 arg3 coma Arg4 arg4 coma Arg5 arg5 coma Arg6 arg6
#define VARIABLE_ARGS_USE arg1 coma arg2 coma arg3 coma arg4 coma arg5 coma arg6
*/
#include INCLUDE_PATH

#undef VARIABLE_NUM_ARGS
#define VARIABLE_NUM_ARGS 5

#undef VARIABLE_ARGS
#undef VARIABLE_ARGS_NODEFAULT
#undef VARIABLE_ARGS_DECL
#undef VARIABLE_ARGS_INVERTED
#undef VARIABLE_ARGS_IMPL
#undef VARIABLE_ARGS_USE

#define VARIABLE_ARGS class Arg1 coma class Arg2 coma class Arg3 coma class Arg4 coma class Arg5 
//#define VARIABLE_ARGS_NODEFAULT class Arg1 coma class Arg2 coma class Arg3 coma class Arg4 coma class Arg5
#define VARIABLE_ARGS_INVERTED class Arg5 coma class Arg4 coma class Arg3 coma class Arg2 coma class Arg1
#define VARIABLE_ARGS_DECL Arg1 coma Arg2 coma Arg3  coma Arg4 coma Arg5
#define VARIABLE_ARGS_IMPL Arg1 arg1 coma Arg2 arg2 coma Arg3 arg3 coma Arg4 arg4 coma Arg5 arg5
#define VARIABLE_ARGS_USE arg1 coma arg2 coma arg3 coma arg4 coma arg5

#include INCLUDE_PATH

#undef VARIABLE_NUM_ARGS


#define VARIABLE_NUM_ARGS 4

#undef VARIABLE_ARGS
#undef VARIABLE_ARGS_NODEFAULT
#undef VARIABLE_ARGS_DECL
#undef VARIABLE_ARGS_INVERTED
#undef VARIABLE_ARGS_IMPL
#undef VARIABLE_ARGS_USE

#define VARIABLE_ARGS class Arg1 coma class Arg2 coma class Arg3 coma class Arg4
#define VARIABLE_ARGS_DECL Arg1 coma Arg2 coma Arg3 coma Arg4
#define VARIABLE_ARGS_INVERTED class Arg4 coma class Arg3 coma class Arg2 coma class Arg1
#define VARIABLE_ARGS_IMPL Arg1 arg1 coma Arg2 arg2 coma Arg3 arg3 coma Arg4 arg4
#define VARIABLE_ARGS_USE arg1 coma arg2 coma arg3 coma arg4

#include INCLUDE_PATH

#undef VARIABLE_NUM_ARGS



#define VARIABLE_NUM_ARGS 3

#undef VARIABLE_ARGS
#undef VARIABLE_ARGS_NODEFAULT
#undef VARIABLE_ARGS_DECL
#undef VARIABLE_ARGS_INVERTED
#undef VARIABLE_ARGS_IMPL
#undef VARIABLE_ARGS_USE

#define VARIABLE_ARGS class Arg1 coma class Arg2 coma class Arg3 
#define VARIABLE_ARGS_DECL Arg1 coma Arg2 coma Arg3 
#define VARIABLE_ARGS_INVERTED class Arg3 coma class Arg2 coma class Arg1
#define VARIABLE_ARGS_IMPL Arg1 arg1 coma Arg2 arg2 coma Arg3 arg3 
#define VARIABLE_ARGS_USE arg1 coma arg2 coma arg3 


#include INCLUDE_PATH

#undef VARIABLE_NUM_ARGS


#define VARIABLE_NUM_ARGS 2

#undef VARIABLE_ARGS
#undef VARIABLE_ARGS_NODEFAULT
#undef VARIABLE_ARGS_DECL
#undef VARIABLE_ARGS_INVERTED
#undef VARIABLE_ARGS_IMPL
#undef VARIABLE_ARGS_USE

#define VARIABLE_ARGS class Arg1 coma class Arg2
#define VARIABLE_ARGS_DECL Arg1 coma Arg2 
#define VARIABLE_ARGS_INVERTED class Arg2 coma class Arg1
#define VARIABLE_ARGS_IMPL Arg1 arg1 coma Arg2 arg2
#define VARIABLE_ARGS_USE arg1 coma arg2 


#include INCLUDE_PATH

#undef VARIABLE_NUM_ARGS


#undef VARIABLE_ARGS
#undef VARIABLE_ARGS_NODEFAULT
#undef VARIABLE_ARGS_DECL
#undef VARIABLE_ARGS_INVERTED
#undef VARIABLE_ARGS_IMPL
#undef VARIABLE_ARGS_USE

#define VARIABLE_NUM_ARGS 1
#define VARIABLE_ARGS class Arg1 
#define VARIABLE_ARGS_DECL Arg1 
#define VARIABLE_ARGS_INVERTED class Arg1
#define VARIABLE_ARGS_IMPL Arg1 arg1 
#define VARIABLE_ARGS_USE arg1 

#include INCLUDE_PATH

#undef VARIABLE_NUM_ARGS
