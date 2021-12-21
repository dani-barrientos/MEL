#pragma once
#include <mpl/binary.h>
using mpl::binary;

#ifdef WIN32
	#include <Windows.h>
#elif defined (_MACOSX) || defined(_IOS) || defined(_ANDROID)
	#include <pthread.h>
#endif 
/**
* common definitions used by threads 
*/
namespace core
{
	/**
	* Thread priority enumeration.
	*/
	enum ThreadPriority {
		TP_LOWEST=0x01, /**< Lowest priority */
		TP_LOW=0x02,	/**< Low priority */
		TP_NORMAL=0x03,	/**< Normal/standard priority. This is the default priority new threads are assigned if none is specified */
		TP_HIGH=0x04,	/**< High priority. Use with caution. */
		TP_HIGHEST=0x05 /**< Highest/realtime priority. May cause the system to become irresponsive while the thread is running. */
	};
	enum EThreadState { 
		THREAD_INIT = binary<1>::value,
		THREAD_RUNNING  = binary<10>::value,
		THREAD_SUSPENDED = binary<100>::value,
		THREAD_FINISHING = binary<1000>::value, 
		THREAD_FINISHING_DONE = binary<10000>::value,
		THREAD_FINISHED = binary<100000>::value
	};

#ifdef WIN32
	typedef DWORD ThreadId;
#endif
#if defined (_MACOSX) || defined(_IOS) || defined(_ANDROID)
	typedef pthread_t ThreadId;
#elif defined( _AIRPLAY)
	typedef int ThreadId; //TODO
#endif

	/**
	* get handle of the current executing thread
	*/
	static inline ThreadId getCurrentThreadId()
	{
	#ifdef _WINDOWS
		return GetCurrentThreadId();
	#elif defined (_MACOSX) || defined(_IOS) || defined(_ANDROID)
		return pthread_self();
	#elif defined (_AIRPLAY)
		return 0;
	#endif
	}
}