#pragma once
#include <FoundationLibType.h>
#include <core/CriticalSection.h>

/**
* policy for thread-safe access to singleton
*/
namespace core
{
	template <class T> class Singleton_Multithread_Policy
	{
	private:
		template <class U>
		class Locker
		{
		public:
			Locker()
			{
				mCs.enter();
			}
			~Locker()
			{
				mCs.leave();
			}
		private:
			static CriticalSection mCs;
		};
	public:
		//typedef T volatile VolatileType;  TODO problemas con SmartPtr
		typedef T VolatileType; 
		typedef Locker<T> Lock;
	
	};
	template <class T> template <class U> CriticalSection Singleton_Multithread_Policy<T>::Locker<U>::mCs;
}