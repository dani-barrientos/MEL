#pragma once
#include <mpl/typelist/TypeList.h>
#include <mpl/TypeTraits.h>
using mel::mpl::typelist::TypeList;

#include <mpl/typelist/Sort.h>
using mel::mpl::typelist::Sort;

#define OPTIMIZE_TUPLE_SIZE 1
namespace mel
{
	namespace mpl
	{
		/**
		* @class Tuple
		* Represent a tuple of types.
		* Example: Tuple<float,int,char> t( 6.7f,5,'a');
		* Element acess through get function:
		*       float a = t.get<0>
		*       int b = t.get<1>
		*       char c = t.get<2>
		*/

		using mel::mpl::typelist::Element;
		using mel::mpl::typelist::Length;
		namespace _private
		{
				template <class TListPos,class TListSorted,int size> class Tuple_Base;
				template <class T,int n> class Component;
			#define MAKE_COMPONENT( N ) \
				template <class T> class Component<T,N> \
				{ \
					typedef typename TypeTraits< T >::ParameterType TArgRef; \
				protected: \
						Component(){}\
						Component( TArgRef arg):mArg( arg ){}; \
						T mArg; \
				};

				MAKE_COMPONENT(0)
				MAKE_COMPONENT(1)
				MAKE_COMPONENT(2)
				MAKE_COMPONENT(3)
				MAKE_COMPONENT(4)
				MAKE_COMPONENT(5)
				MAKE_COMPONENT(6)
				MAKE_COMPONENT(7)
			#if OPTIMIZE_TUPLE_SIZE == 1

			//TODO FORMA CHAPUCERA TEMPORAL
			//helper class to sort by size
				template <class T> struct PackCondition
				{
					enum {result = -(int)sizeof(T)};
				};
			#else
				template <class T> struct PackCondition
				{
					enum {result = 0};
				};
			#endif
			

			//TODO resolver el tema de tipo void
				/*template <> class Component1<void>
				{

				};*/
				//one element tuple
				template <class TTypes,class SortedTypes> class Tuple_Base<TTypes,SortedTypes,1> :
						public Component<typename Element<typename SortedTypes::Sorted,0,true>::Result,0 >
				{
					protected:

						typedef typename Element<TTypes,0,true>::Result TArg;

						typedef typename TypeTraits< TArg >::ParameterType TArgRef;
						Tuple_Base(){}
						Tuple_Base( TArgRef arg ): Component<TArg,Element<typename SortedTypes::Indexes,0,true>::Result::value>( arg ){}
				};

			template <class TTypes,class SortedTypes> class Tuple_Base<TTypes,SortedTypes,2> :
						public Component<typename Element<typename SortedTypes::Sorted,0,true>::Result,0 >,
						public Component<typename Element<typename SortedTypes::Sorted,1,true>::Result,1 >
				{
					protected:

						typedef typename Element<TTypes,0,true>::Result TArg1;
						typedef typename Element<TTypes,1,true>::Result TArg2;

						typedef typename TypeTraits< TArg1 >::ParameterType TArg1Ref;
						typedef typename TypeTraits< TArg2 >::ParameterType TArg2Ref;
						Tuple_Base(){}
						Tuple_Base( TArg1Ref arg1, TArg2Ref arg2 ): Component<TArg1,Element<typename SortedTypes::Indexes,0,true>::Result::value>( arg1 ),
																	Component<TArg2,Element<typename SortedTypes::Indexes,1,true>::Result::value>( arg2 ){}
				};

			template <class TTypes,class SortedTypes> class Tuple_Base<TTypes,SortedTypes,3> :
						public Component<typename Element<typename SortedTypes::Sorted,0,true>::Result,0 >,
						public Component<typename Element<typename SortedTypes::Sorted,1,true>::Result,1 >,
						public Component<typename Element<typename SortedTypes::Sorted,2,true>::Result,2 >
				{
					protected:

						typedef typename Element<TTypes,0,true>::Result TArg1;
						typedef typename Element<TTypes,1,true>::Result TArg2;
						typedef typename Element<TTypes,2,true>::Result TArg3;

						typedef typename TypeTraits< TArg1 >::ParameterType TArg1Ref;
						typedef typename TypeTraits< TArg2 >::ParameterType TArg2Ref;
						typedef typename TypeTraits< TArg3 >::ParameterType TArg3Ref;

						Tuple_Base(){}
						Tuple_Base( TArg1Ref arg1, TArg2Ref arg2,TArg3Ref arg3 ):
									Component<TArg1,Element<typename SortedTypes::Indexes,0,true>::Result::value>( arg1 ),
									Component<TArg2,Element<typename SortedTypes::Indexes,1,true>::Result::value>( arg2 ),
									Component<TArg3,Element<typename SortedTypes::Indexes,2,true>::Result::value>( arg3 ){}
				};

			template <class TTypes,class SortedTypes> class Tuple_Base<TTypes,SortedTypes,4> :
						public Component<typename Element<typename SortedTypes::Sorted,0,true>::Result,0 >,
						public Component<typename Element<typename SortedTypes::Sorted,1,true>::Result,1 >,
						public Component<typename Element<typename SortedTypes::Sorted,2,true>::Result,2 >,
						public Component<typename Element<typename SortedTypes::Sorted,3,true>::Result,3 >
				{
					protected:

						typedef typename Element<TTypes,0,true>::Result TArg1;
						typedef typename Element<TTypes,1,true>::Result TArg2;
						typedef typename Element<TTypes,2,true>::Result TArg3;
						typedef typename Element<TTypes,3,true>::Result TArg4;

						typedef typename TypeTraits< TArg1 >::ParameterType TArg1Ref;
						typedef typename TypeTraits< TArg2 >::ParameterType TArg2Ref;
						typedef typename TypeTraits< TArg3 >::ParameterType TArg3Ref;
						typedef typename TypeTraits< TArg4 >::ParameterType TArg4Ref;

						Tuple_Base(){}
						Tuple_Base( TArg1Ref arg1, TArg2Ref arg2,TArg3Ref arg3,TArg4Ref arg4):
									Component<TArg1,Element<typename SortedTypes::Indexes,0,true>::Result::value>( arg1 ),
									Component<TArg2,Element<typename SortedTypes::Indexes,1,true>::Result::value>( arg2 ),
									Component<TArg3,Element<typename SortedTypes::Indexes,2,true>::Result::value>( arg3 ),
									Component<TArg4,Element<typename SortedTypes::Indexes,3,true>::Result::value>( arg4 ){}
				};
					template <class TTypes,class SortedTypes> class Tuple_Base<TTypes,SortedTypes,5> :
						public Component<typename Element<typename SortedTypes::Sorted,0,true>::Result,0 >,
						public Component<typename Element<typename SortedTypes::Sorted,1,true>::Result,1 >,
						public Component<typename Element<typename SortedTypes::Sorted,2,true>::Result,2 >,
						public Component<typename Element<typename SortedTypes::Sorted,3,true>::Result,3 >,
						public Component<typename Element<typename SortedTypes::Sorted,4,true>::Result,4 >
				{
					protected:

						typedef typename Element<TTypes,0,true>::Result TArg1;
						typedef typename Element<TTypes,1,true>::Result TArg2;
						typedef typename Element<TTypes,2,true>::Result TArg3;
						typedef typename Element<TTypes,3,true>::Result TArg4;
						typedef typename Element<TTypes,4,true>::Result TArg5;

						typedef typename TypeTraits< TArg1 >::ParameterType TArg1Ref;
						typedef typename TypeTraits< TArg2 >::ParameterType TArg2Ref;
						typedef typename TypeTraits< TArg3 >::ParameterType TArg3Ref;
						typedef typename TypeTraits< TArg4 >::ParameterType TArg4Ref;
						typedef typename TypeTraits< TArg5 >::ParameterType TArg5Ref;

						Tuple_Base(){}
						Tuple_Base( TArg1Ref arg1, TArg2Ref arg2,TArg3Ref arg3,TArg4Ref arg4,
								TArg5Ref arg5):
									Component<TArg1,Element<typename SortedTypes::Indexes,0,true>::Result::value>( arg1 ),
									Component<TArg2,Element<typename SortedTypes::Indexes,1,true>::Result::value>( arg2 ),
									Component<TArg3,Element<typename SortedTypes::Indexes,2,true>::Result::value>( arg3 ),
									Component<TArg4,Element<typename SortedTypes::Indexes,3,true>::Result::value>( arg4 ),
									Component<TArg5,Element<typename SortedTypes::Indexes,4,true>::Result::value>( arg5 ){}
				};
				template <class TTypes,class SortedTypes> class Tuple_Base<TTypes,SortedTypes,6> :
						public Component<typename Element<typename SortedTypes::Sorted,0,true>::Result,0 >,
						public Component<typename Element<typename SortedTypes::Sorted,1,true>::Result,1 >,
						public Component<typename Element<typename SortedTypes::Sorted,2,true>::Result,2 >,
						public Component<typename Element<typename SortedTypes::Sorted,3,true>::Result,3 >,
						public Component<typename Element<typename SortedTypes::Sorted,4,true>::Result,4 >,
						public Component<typename Element<typename SortedTypes::Sorted,5,true>::Result,5 >
				{
					protected:

						typedef typename Element<TTypes,0,true>::Result TArg1;
						typedef typename Element<TTypes,1,true>::Result TArg2;
						typedef typename Element<TTypes,2,true>::Result TArg3;
						typedef typename Element<TTypes,3,true>::Result TArg4;
						typedef typename Element<TTypes,4,true>::Result TArg5;
						typedef typename Element<TTypes,5,true>::Result TArg6;

						typedef typename TypeTraits< TArg1 >::ParameterType TArg1Ref;
						typedef typename TypeTraits< TArg2 >::ParameterType TArg2Ref;
						typedef typename TypeTraits< TArg3 >::ParameterType TArg3Ref;
						typedef typename TypeTraits< TArg4 >::ParameterType TArg4Ref;
						typedef typename TypeTraits< TArg5 >::ParameterType TArg5Ref;
						typedef typename TypeTraits< TArg6 >::ParameterType TArg6Ref;

						Tuple_Base(){}
						Tuple_Base( TArg1Ref arg1, TArg2Ref arg2,TArg3Ref arg3,TArg4Ref arg4,
								TArg5Ref arg5,TArg6Ref arg6):
									Component<TArg1,Element<typename SortedTypes::Indexes,0,true>::Result::value>( arg1 ),
									Component<TArg2,Element<typename SortedTypes::Indexes,1,true>::Result::value>( arg2 ),
									Component<TArg3,Element<typename SortedTypes::Indexes,2,true>::Result::value>( arg3 ),
									Component<TArg4,Element<typename SortedTypes::Indexes,3,true>::Result::value>( arg4 ),
									Component<TArg5,Element<typename SortedTypes::Indexes,4,true>::Result::value>( arg5 ),
									Component<TArg6,Element<typename SortedTypes::Indexes,5,true>::Result::value>( arg6 ){}
				};
					template <class TTypes,class SortedTypes> class Tuple_Base<TTypes,SortedTypes,7> :
						public Component<typename Element<typename SortedTypes::Sorted,0,true>::Result,0 >,
							public Component<typename Element<typename SortedTypes::Sorted,1,true>::Result,1 >,
							public Component<typename Element<typename SortedTypes::Sorted,2,true>::Result,2 >,
							public Component<typename Element<typename SortedTypes::Sorted,3,true>::Result,3 >,
							public Component<typename Element<typename SortedTypes::Sorted,4,true>::Result,4 >,
							public Component<typename Element<typename SortedTypes::Sorted,5,true>::Result,5 >,
							public Component<typename Element<typename SortedTypes::Sorted,6,true>::Result,6 >
						{
						protected:

							typedef typename Element<TTypes,0,true>::Result TArg1;
							typedef typename Element<TTypes,1,true>::Result TArg2;
							typedef typename Element<TTypes,2,true>::Result TArg3;
							typedef typename Element<TTypes,3,true>::Result TArg4;
							typedef typename Element<TTypes,4,true>::Result TArg5;
							typedef typename Element<TTypes,5,true>::Result TArg6;
							typedef typename Element<TTypes,6,true>::Result TArg7;

							typedef typename TypeTraits< TArg1 >::ParameterType TArg1Ref;
							typedef typename TypeTraits< TArg2 >::ParameterType TArg2Ref;
							typedef typename TypeTraits< TArg3 >::ParameterType TArg3Ref;
							typedef typename TypeTraits< TArg4 >::ParameterType TArg4Ref;
							typedef typename TypeTraits< TArg5 >::ParameterType TArg5Ref;
							typedef typename TypeTraits< TArg6 >::ParameterType TArg6Ref;
							typedef typename TypeTraits< TArg7 >::ParameterType TArg7Ref;

							Tuple_Base(){}
							Tuple_Base( TArg1Ref arg1, TArg2Ref arg2,TArg3Ref arg3,TArg4Ref arg4,
								TArg5Ref arg5,TArg6Ref arg6,TArg7Ref arg7):
							Component<TArg1,Element<typename SortedTypes::Indexes,0,true>::Result::value>( arg1 ),
								Component<TArg2,Element<typename SortedTypes::Indexes,1,true>::Result::value>( arg2 ),
								Component<TArg3,Element<typename SortedTypes::Indexes,2,true>::Result::value>( arg3 ),
								Component<TArg4,Element<typename SortedTypes::Indexes,3,true>::Result::value>( arg4 ),
								Component<TArg5,Element<typename SortedTypes::Indexes,4,true>::Result::value>( arg5 ),
								Component<TArg6,Element<typename SortedTypes::Indexes,5,true>::Result::value>( arg6 ),
								Component<TArg6,Element<typename SortedTypes::Indexes,6,true>::Result::value>( arg7 ){}
						};

		} //End namespace private


		template <class TTypes > class Tuple : public _private::Tuple_Base< TTypes,typename mel::mpl::typelist::Sort<TTypes,_private::PackCondition>::Result, Length<TTypes>::result >
		{
			enum{ totallength = Length<TTypes>::result};
			typedef typename mel::mpl::typelist::Sort<TTypes,_private::PackCondition>::Result SortedTypes;
			typedef _private::Tuple_Base< TTypes,SortedTypes, totallength > BaseClass;

			typedef typename Element<TTypes,0,true>::Result TArg1;
			typedef typename Element<TTypes,1,true>::Result TArg2;
			typedef typename Element<TTypes,2,true>::Result TArg3;
			typedef typename Element<TTypes,3,true>::Result TArg4;
			typedef typename Element<TTypes,4,true>::Result TArg5;
			typedef typename Element<TTypes,5,true>::Result TArg6;
			typedef typename Element<TTypes,6,true>::Result TArg7;


			typedef typename TypeTraits< TArg1 >::ParameterType TArg1Ref;
			typedef typename TypeTraits< TArg2 >::ParameterType TArg2Ref;
			typedef typename TypeTraits< TArg3 >::ParameterType TArg3Ref;
			typedef typename TypeTraits< TArg4 >::ParameterType TArg4Ref;
			typedef typename TypeTraits< TArg5 >::ParameterType TArg5Ref;
			typedef typename TypeTraits< TArg6 >::ParameterType TArg6Ref;
			typedef typename TypeTraits< TArg7 >::ParameterType TArg7Ref;

			public:
				Tuple(){};
				Tuple( TArg1Ref arg1 ):BaseClass( arg1 ){}
				Tuple( TArg1Ref arg1,TArg2Ref arg2 ):BaseClass( arg1, arg2 ){}
				Tuple( TArg1Ref arg1,TArg2Ref arg2, TArg3Ref arg3 ):BaseClass( arg1, arg2,arg3 ){}
				Tuple( TArg1Ref arg1,TArg2Ref arg2, TArg3Ref arg3, TArg4Ref arg4 ):BaseClass( arg1, arg2,arg3,arg4 ){}
				Tuple( TArg1Ref arg1,TArg2Ref arg2, TArg3Ref arg3, TArg4Ref arg4, TArg5Ref arg5):BaseClass( arg1, arg2,arg3,arg4,arg5 ){}
				Tuple( TArg1Ref arg1,TArg2Ref arg2, TArg3Ref arg3, TArg4Ref arg4, TArg5Ref arg5,TArg6Ref arg6):BaseClass( arg1, arg2,arg3,arg4,arg5,arg6 ){}
				Tuple( TArg1Ref arg1,TArg2Ref arg2, TArg3Ref arg3, TArg4Ref arg4, TArg5Ref arg5,TArg6Ref arg6,TArg7Ref arg7):BaseClass( arg1, arg2,arg3,arg4,arg5,arg6,arg7 ){}

				//component selection (0 base index)

			//TODO esto de la referencia no puede estar bien tan a la ligera. �QU� pasa con los atributos que ya son referencia?
				template <int n> inline
				typename Element<TTypes,n,true>::Result& get()
				{
					typedef typename Element<typename SortedTypes::Indexes,n,true>::Result CurrentElement;
					return _private::Component<typename Element<TTypes,n,true>::Result,CurrentElement::value>::mArg;
				}
				//const version
				template <int n> inline
					const typename Element<TTypes,n,true>::Result& get() const
				{
					typedef typename Element<typename SortedTypes::Indexes,n,true>::Result CurrentElement;
					return _private::Component<typename Element<TTypes,n,true>::Result,CurrentElement::value>::mArg;
				}

				/*
				NO ME CONVENCE EL SET YA QUE NO ES NECESARIO
				template <int n> inline void set( typename TypeTraits< typename Element<TTypes,n,true>::Result>::ParameterType arg )
				{
					typedef typename Element<typename SortedTypes::Indexes,n,true>::Result CurrentElement;
					Component<typename Element<TTypes,n,true>::Result,CurrentElement::value>::mArg = arg;
				}
				*/



		};

		/*
		NO USAR TODAVIA POR TEMAS DE REFERENCIAS EN PARAMETROS
		template <class T1> Tuple< TYPELIST( T1 ) > makeTuple( T1 arg)
		{
			return Tuple< TYPELIST( T1 ) >( arg );
		}
		template <class T1,class T2> Tuple< TYPELIST( T1, T2 ) > makeTuple( T1 arg1, T2 arg2)
		{
			return Tuple< TYPELIST( T1,T2 ) >( arg1,arg2 );
		}

		template <class T1,class T2,class T3> Tuple< TYPELIST( T1, T2,T3 ) > makeTuple( T1 arg1, T2 arg2,T3 arg3)
		{
			return Tuple< TYPELIST( T1,T2,T3 ) >( arg1,arg2,arg3 );
		}
		template <class T1,class T2,class T3,class T4> Tuple< TYPELIST( T1, T2,T3,T4 ) > makeTuple( T1 arg1, T2 arg2,T3 arg3,T4 arg4 )
		{
			return Tuple< TYPELIST( T1,T2,T3,T4 ) >( arg1,arg2,arg3,arg4 );
		}
		template <class T1,class T2,class T3,class T4,class T5> Tuple< TYPELIST( T1, T2,T3,T4,T5 ) > makeTuple( T1 arg1, T2 arg2,T3 arg3,T4 arg4,T5 arg5 )
		{
			return Tuple< TYPELIST( T1,T2,T3,T4,T5 ) >( arg1,arg2,arg3,arg4,arg5 );
		}
		template <class T1,class T2,class T3,class T4,class T5,class T6> Tuple< TYPELIST( T1, T2,T3,T4,T5,T6 ) > makeTuple( T1 arg1, T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6 )
		{
			return Tuple< TYPELIST( T1,T2,T3,T4,T5,T6 ) >( arg1,arg2,arg3,arg4,arg5,arg6 );
		}
	*/

	}
}