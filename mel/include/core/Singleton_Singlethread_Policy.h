#pragma once
/**
* policy for thread-safe access to singleton
*/
namespace mel
{
	namespace core
	{
		template <class T> class Singleton_Singlethread_Policy
		{
		public:
			class Lock
			{
			public:
				Lock(){} //to avoid warning for unused variable
			};

			typedef T VolatileType;
		private:

		};
	}
}