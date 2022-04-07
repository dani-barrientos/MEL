#pragma once
#include <mpl/typelist/TypeList.h>
#include <mpl/_If.h>
#include <limits.h>

namespace mel
{
	namespace mpl
	{
		namespace typelist
		{
			//coger el mejor que cumpla una condicion
			namespace _private
			{
				template <class , template <class> class ,int ,int ,int > struct FindBestPos;
				template <class H, class T, template <class> class Condition,int currentPos,int candidatePos,int candidateValue> struct FindBestPos< TypeList<H,T>, Condition,currentPos,candidatePos,candidateValue >
				{
				private:
					enum {isBetter = (Condition<H>::result > candidateValue)};
					//typedef typename _if<isBetter,H,Candidate>::Result currentCandidatePos;
					enum { currentCandidatePos = _ifInt<isBetter,currentPos,candidatePos>::result };
					enum { newValue = _ifInt<isBetter,Condition<H>::result,candidateValue>::result};
				public:
					enum { result = FindBestPos<T,Condition,currentPos+1,currentCandidatePos,newValue>::result };
				};
				template <template <class> class Condition,int currentPos,int candidatePos,int candidateValue> struct FindBestPos< NullType, Condition,currentPos,candidatePos,candidateValue >
				{
					enum { result = candidatePos };
				};

			}

			//Condition::result sera un entero: a mayor numero,mejor la condicion
			template <class TList, template <class> class Condition> struct FindBest
			{
			private:
				enum {candidateValue=INT_MIN}; //no puedo usar numeric_limits porque no es const
			public:
				enum { result = _private::FindBestPos< TList,Condition,0,0,candidateValue >::result };
			};

		}
	}
}