#pragma once
/**
* @file 
* @brief Typical Traits for types
*/

#include <mpl/BasicTypes.h>
using mel::mpl::NullType;

#include <mpl/_If.h>
using mel::mpl::_if;
#include <mpl/CommonTypes.h>
#include <type_traits>  //TODO tratar de no meterlo porque pa eso ya este archivo sobrar�a en gran parte

namespace mel
{
	namespace mpl
	{
		template <class U>
		struct PointerTraits
		{
			enum{ result = false };
			typedef NullType	PointeeType; //tipo del contenido
		};
		template <class U>
		struct PointerTraits<U*>
		{
			enum{ result = true };
			typedef U	PointeeType; //tipo del contenido
		};
		template <class U>
		struct PointerTraits<U*&>
		{
			enum { result = true };
			typedef U	PointeeType; //tipo del contenido
		};
		template <class U>
		struct PointerToMemberTraits
		{
			enum{ result = false };
		};
		template <class U,class V>
		struct PointerToMemberTraits< U V::*>
		{
			enum{ result = true };
		};
		//traits to know if T is c function. Done without much time, sure should be better done
		template <class U>
		struct FunctionPointerTraits
		{
			enum{result=false};
		};
		template <class U>
		struct FunctionPointerTraits< U(void) >
		{
			enum { result = true };
		};
		template <class U, class Arg1>
		struct FunctionPointerTraits< U(Arg1) >
		{
			enum { result = true };
		};
		template <class U, class Arg1, class Arg2>
		struct FunctionPointerTraits< U(Arg1, Arg2) >
		{
			enum { result = true };
		};
		template <class U, class Arg1, class Arg2,class Arg3>
		struct FunctionPointerTraits< U(Arg1, Arg2,Arg3) >
		{
			enum { result = true };
		};
		template <class U, class Arg1, class Arg2, class Arg3,class Arg4>
		struct FunctionPointerTraits< U(Arg1, Arg2, Arg3, Arg4) >
		{
			enum { result = true };
		};
		template <class U, class Arg1, class Arg2, class Arg3, class Arg4,class Arg5>
		struct FunctionPointerTraits< U(Arg1, Arg2, Arg3, Arg4, Arg5) >
		{
			enum { result = true };
		};
		template <class U, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6>
		struct FunctionPointerTraits< U(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) >
		{
			enum { result = true };
		};
		template <class U, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6,class Arg7>
		struct FunctionPointerTraits< U(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7) >
		{
			enum { result = true };
		};
		template <class U, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7,class Arg8>
		struct FunctionPointerTraits< U(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7,Arg8) >
		{
			enum { result = true };
		};
		template <class U> struct ReferenceTraits
		{
			enum { result = false };
			typedef const U& ReferenceType;
		};
		template <class U> struct ReferenceTraits<U&>
		{
			enum { result = true };
			typedef U& ReferenceType;
		};
		template <> struct ReferenceTraits<void>
		{
			enum { result = false };
			typedef void ReferenceType;
		};

		template <class U>
		struct UnConst
		{
			typedef U Result;
			enum {isConst = false};
		};
		template <class U>
		struct UnConst<const U>
		{
			typedef U Result;
			enum {isConst = true};
		};
		template <class U>
		struct UnConst<const U&>
		{
			typedef U& Result;
			enum {isConst = true};
		};
		//desreferencia el tipo
		template <class U> struct UnRef
		{
			typedef U Result;
		};
		template <class U> struct UnRef<U&>
		{
			typedef U Result;
		};
		template <class U>
		struct IsConst
		{
			enum { result = false};
		};
		template <class U>
		struct IsConst<U const>
		{
			enum { result = true};
		};

		template <class U>
		struct IntTraits
		{
			enum { result = false };
		};
		template <>
		struct IntTraits<int>
		{
			enum { result = true };
		};
		template <class U>
		struct UIntTraits
		{
			enum { result = false };
		};
		template <>
		struct UIntTraits<unsigned int>
		{
			enum { result = true };
		};
		template <class U>
		struct BoolTraits
		{
			enum { result = false };
		};
		template <>
		struct BoolTraits<bool>
		{
			enum { result = true };
		};
		template <class U>
		struct CharTraits
		{
			enum { result = false };
		};
		template <>
		struct CharTraits<char>
		{
			enum { result = true };
		};
		template <class U>
		struct UCharTraits
		{
			enum { result = false };
		};
		template <>
		struct UCharTraits<unsigned char>
		{
			enum { result = true };
		};
		template <class U>
		struct FloatTraits
		{
			enum { result = false };
		};
		template <>
		struct FloatTraits<float>
		{
			enum { result = true };
		};
		template <class U>
		struct DoubleTraits
		{
			enum { result = false };
		};
		template <>
		struct DoubleTraits<double>
		{
			enum { result = true };
		};

		template <class U>
		struct VoidTraits
		{
			enum { result = false };
		};
		template <>
		struct VoidTraits< void >
		{
			enum { result = true };
		};

		template<class U>
		struct VolatileTraits
		{
			typedef U Result;
		};
		template <class U>
		struct VolatileTraits<U volatile>
		{
			typedef U Result;
		};

		//COMPRUEBA SI LA CLASE TIENE DEFINIDO EL operator==
		//TODAVIA MUY TEMPORAL HASTA TENER METODO GENERALIZADO
		//!@todo todav�a no valido cuando el operator pertenece a una clase base de T
	/*	template <class T> struct ComparableTraits
		{
			template <class U, bool (U::*)(const U&) const> struct AUXSTRUCT{};
			typedef char Small;
			class Big{ char dummy[2];};
			template <class U> static Small check( AUXSTRUCT< U,&U::operator== >* );
			template <class U> static Big check( ... );

			enum{ Result = (  TypeTraits<T>::isIntegral ||
					TypeTraits<T>::isPointerToMember ||
					TypeTraits<T>::isFloating || TypeTraits<T>::isPointer
					|| sizeof( check<T>(0)) == sizeof(Small) ) };

		};
		*/
		//sigue habiendo el problema en XCode del == deleted
		//template<class T1, class T2> Small operator==(const T1&, const T2&); //define default operator==
		Small operator==(const AnyType&, const AnyType&); //define default operator==
	//	template<class T> Small operator==(const T&, const T&); //define default operator==
		template <class U>
		struct ComparableTraits
		{
			static const U& x;
			static Small comparablecheck( const Small& );
			static Big comparablecheck( ... );
			enum { result = (sizeof(comparablecheck(x == x)) == sizeof(Big)) };
		};
	/*	template <class U>
		struct ComparableTraits<U *>
		{
			enum { result = true };
		};*/
		
	/*	template <class U> struct ComparableTraits<U,false>
		{
			enum { result = false };
			//enum { result = (sizeof(comparablecheck( x.operator==(x))) == sizeof(Big)) };
		};*/
		template <class T>
		struct ClassTraits
		{
			template <typename U> static Small isClass( int U::* );
			template <typename U> static Big isClass( ... );
			enum{ result = (sizeof( isClass<T>(0) ) == sizeof(Small) )};
		};

		template <class T>
		class TypeTraits
		{

		public:
			enum{ isPointer = PointerTraits<T>::result };
			enum { isPointerToMember = PointerToMemberTraits<T>::result };
			enum { isInt = IntTraits<T>::result };
			enum { isUInt = UIntTraits<T>::result };
			enum { isBool = BoolTraits<T>::result };
			enum { isChar = CharTraits<T>::result };
			enum { isUChar = UCharTraits<T>::result };
			enum { isIntegral = isInt || isUInt };
			enum { isByte = isChar || isUChar || isBool };
			enum { isFloat = FloatTraits<T>::result };
			enum { isDouble = DoubleTraits<T>::result };
			enum { isFloating = isFloat ||isDouble };

			enum { isArith = isIntegral || isFloating };
			enum { isVoid = VoidTraits<T>::result };
			enum { isFundamental = isArith ||isVoid };
			enum { isClass = ClassTraits<T>::result };
			//enum { isConst = UnConst<T>::isConst };
			enum { isConst = IsConst<T>::result };
			enum { isReference = ReferenceTraits<T>::result };
			enum {isFunction = FunctionPointerTraits<T>::result};
			typedef typename PointerTraits<T>::PointeeType		PointeeType;
			typedef typename UnConst<T>::Result					NoConst;

		//quiero una forma de que me de la referencia sin que me ponga el const
			//typedef typename _if< isByte || isPointer || isArith || isPointerToMember || isReference ||isPointer,T,typename ReferenceTraits<T>::ReferenceType >::Result		ParameterType;  //mejor parametro para pasar a funciones. Recordad que pone el const en las referencias
			//metodo no totalmente preciso pero valido casi siempre
			typedef typename _if< (isClass && sizeof(typename std::decay<T>::type)<= sizeof(int))||!isClass,T,typename ReferenceTraits<T>::ReferenceType >::Result		ParameterType;  //mejor parametro para pasar a funciones. Recordad que pone el const en las referencias
		// typedef typename _if< (isClass && sizeof(T)<= sizeof(int))||!isClass,T,T& >::Result		ReturnType; //para retorno de funciones. CREO QUE NO LO TENGO BIEN;LO HICE A PRISA
			//TODO no v�lido para T == void
			// cuidado si tenemos un struct vacio da sizeof = 1, pero llamar� a constructor de copia(si hay...)typedef typename _if< sizeof(T) <= sizeof(int),T,typename ReferenceTraits<T>::ReferenceType >::Result		ParameterType;

			typedef typename UnRef<T>::Result	UnReferenced;

		};
	}
	//::mpl::Small operator==(const mel::mpl::AnyType&, const mel::mpl::AnyType&); //define default operator==
}