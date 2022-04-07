
namespace mel
{
	namespace mpl
	{
	/*
	* macros and template to know if an object has a specific member function
	* usage. Example: suppose you want to know if some Class has a member function int pepito( float,char) const
	* Then you must use macro DECLARE_HASMEMBER as follows:
	*		DECLARE_HASMEMBER( pepito, int,(float,char),const );
	*	First parameter is name function, second is return type, third are functions parameters and fourth is any attribute (const, throw,...)
	* This macro create a new template struct called Haspepito
	* 
	* Once declared this new struct, you can use it. If you have a class called MiClase, you will use it as follows:
	*		Haspepito<MiClase>::Result
	* This is a compiler-time bool constant that you can use to decide to, for example, to use or no to use that function, or execute
	* a different program path, etc
	*
	* @remarks  THIS IS NOT ONLY VALID( yet ) FOR MEMBERS BELONGING TO PARENT CLASSES. So if,in the previous example, MiClase inherits from 
	* MiClase_Base and this class has pepito method then, Haspepito<MiClase>::Result is FALSE (this will be addressed in a future)
	*/
		#define DECLARE_HASMEMBER(func_name,TRet,args, attributes ) \
		template <class T> struct Has##func_name \
		{ \
			template <class U,TRet (U::*) args attributes > struct AUXSTRUCT{};\
			class Big{ char dummy[2];}; \
			typedef char Small; \
			template <class U> static Small check( AUXSTRUCT< U,&U::func_name >* ); \
			template <class U> static Big check( ... ); \
			enum{ Result = sizeof( check<T>(0)) == sizeof(Small)  }; \
		};
	}
}