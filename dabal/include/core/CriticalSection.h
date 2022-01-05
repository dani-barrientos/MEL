#pragma once
#include <DabalLibType.h>
#include <cstddef>
#include <new>
#include <memory>
/**
 * @deprecated 
 * use std::mutex and related instead
 **/
namespace core {
	/**
	 * Platform-independent critical section implementation.
	 * Do I really need to explain anything else here? :P
	 */
	class DABAL_API CriticalSection {
		public:
			CriticalSection():
				mData(::std::make_shared<CriticalSectionData>()) {
			}

			/**
			 * Enter critical section.
			 * If another thread entered the critical section and has not yet leaved it, the calling
			 * thread will be locked until the previous one exits (calls CriticalSection::leave()).
			 * The same thread is allowed to call enter() any number of times without blocking, provided
			 * it then calls leave() the same number of times.
			 * @see leave, tryEnter
			 */
			inline void enter();
			/**
			 * Tries to enter the critical section.
			 * @return true if the section has successfully been entered. false otherwise.
			 */
			inline bool tryEnter();
			/**
			 * Leave the critical section.
			 * Thus, allowing another thread to get unlocked from a CritcalSection::enter call.
			 * Only threads that have already entered the critical section should call this method.
			 * Results will be undefined otherwise.
			 */
			inline void leave();

		private:
			class CriticalSectionData
			{
			public:
				inline CriticalSectionData();
				inline ~CriticalSectionData();
				inline void enter();
				inline bool tryEnter();
				inline void leave();
#ifdef _WINDOWS
				CRITICAL_SECTION mCS;
#endif
#if defined(DABAL_POSIX)
				pthread_mutex_t mCS;
#endif
			};

		protected:
			std::shared_ptr<CriticalSectionData> mData;
	};
	void CriticalSection::enter()
	{
		mData->enter();
	}

	bool CriticalSection::tryEnter() 
	{
		return mData->tryEnter();
	}

	void CriticalSection::leave() 
	{
		mData->leave();
	}
	/**
	* @class Lock
	* @brief Helper class for easy use of the CriticalSections class to lock scopes. You just have to declare a 'Lock' local variable
	* using the pertinent CriticalSection as constructor argument. The ambit will be thread-safe using the given CriticalSection. When
	* the execution goes out of the scope, the CriticalSection will be automatically released.
	* Example:
	*
	*	CriticalSection cs;
	*	void foobar ()
	*	{
	*		Lock lock (cs);
	*		... all method is thread-safe using the mCS CriticalSection...
	*	}
	*/
	class DABAL_API Lock
	{
		public:
			Lock( CriticalSection& cs ): mCs( cs )
			{
				cs.enter();
			};
			~Lock()
			{
				mCs.leave();
			}
		private:

			// Disabled methods and constructors

			Lock();
			Lock(const Lock & cs) : mCs(cs.mCs){}
			
			inline void * operator new (std::size_t size) throw() {return NULL;}
			inline void * operator new (std::size_t size, const std::nothrow_t & nothrow_constant) throw() {return NULL;}
			inline void * operator new (std::size_t size, void* ptr) throw() {return NULL;}
			inline void * operator new[] (std::size_t size) throw() {return NULL;}
			inline void * operator new[] (std::size_t size, const std::nothrow_t & nothrow_constant) throw() {return NULL;}
			inline void * operator new[] (std::size_t size, void* ptr) throw() {return NULL;}

			inline void operator delete (void* ptr) {}
			inline void operator delete (void* ptr, const std::nothrow_t & nothrow_constant) {}
			inline void operator delete (void* ptr, void* voidptr2) {}
			inline void operator delete[] (void* ptr) {}
			inline void operator delete[] (void* ptr, const std::nothrow_t & nothrow_constant) {}
			inline void operator delete[] (void* ptr, void* voidptr2) {}

			CriticalSection& mCs;
	};
}
#ifdef _WINDOWS
#include <core/CriticalSection_win32.inl>
#elif defined(DABAL_POSIX)
#include <core/CriticalSection_posix.inl>
#endif
