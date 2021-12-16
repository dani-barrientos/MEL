///////////////////////////////////////////////////////////
//  State.cpp
//  Implementation of the Class State
//  Created on:      29-mar-2005 10:00:13
///////////////////////////////////////////////////////////

#include <core/State.h>

using core::State;
#include <core/Singleton.h>
using core::Singleton;

//#include <mpl/TypeTraits.h>
//using mpl::TypeTraits;
//#include <core/IRefCount.h>
//using core::IRefCount;
//#include <core/Singleton_Multithread_Policy.h>
//using core::Singleton_Multithread_Policy;
//
//class Clase1 : public Singleton<Clase1,false,false,Singleton_Multithread_Policy>, public IRefCount
//{
//};
//class Clase2 : public IRefCount
//{
//};
State::State(void):
	mElapsedTime(0),
	mSource( 0 )
{

}


State::~State(void)
{
}




