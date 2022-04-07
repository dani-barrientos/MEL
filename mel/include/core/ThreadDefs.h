#pragma once
#include <mpl/binary.h>
using mel::mpl::binary;

#ifdef WIN32
	#include <Windows.h>
#else
	#include <pthread.h>
#endif 
/**
* common definitions used by threads 
*/
namespace mel
{
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

	#ifdef WIN32
		typedef DWORD ThreadId;
	#else
		typedef pthread_t ThreadId;
	#endif

		/**
		* get handle of the current executing thread
		*/
		static inline ThreadId getCurrentThreadId()
		{
		#ifdef _WINDOWS
			return GetCurrentThreadId();
		#else
			return pthread_self();	
		#endif
		}
	}
}