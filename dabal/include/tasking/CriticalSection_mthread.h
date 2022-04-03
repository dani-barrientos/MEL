#pragma once
#include <tasking/Event_mthread.h>

namespace tasking
{
	using ::tasking::Event_mthread;
	/**
	 * @brief A critical section for synchronizing microthreads (AKA \ref ::tasking::Process "Process")
	* not multihread safe
	*/
	class DABAL_API CriticalSection_mthread
	{
	public:
		CriticalSection_mthread();
		/**
		* wait for critical section and enter
		* @return if false, then can't eneter critical section (becausee process kill, for example)
		* @remarks mejorar valor de salida, para tener algo ahora
		*/
		bool enter();
		void leave();
		//TODO try enter
	private:
		Event_mthread mEvent;
		std::shared_ptr<::tasking::Process>	mOwner;
		int	mCount; //number of times owner as acquired section
	};
	class Lock_mthread
	{
	public:
		//! enter critical section. Throw IllegalStateException if can't enter
		Lock_mthread(CriticalSection_mthread& cs);
		~Lock_mthread();
	private:
		CriticalSection_mthread & mCS;
	};

}
