#pragma once
#include <parallelism/Barrier.h>
//#include <atomic> on la version actual que usamos de NDK(r11c en este momento) no compila 
#include <core/CriticalSection.h>
namespace parallelism
{
	/**
	* barrier for a working group.
	* Each barrier is associated with n "workers". When a worker reaches barrier, it should do a set over it.
	* When all workers do de "set" over it, the barrier will be opened, so any working thread/microthread will be woken
	*/
	class SimpleBarrier;
	class SimpleBarrierData : public BarrierData
	{
		friend class ::parallelism::SimpleBarrier;
	private:
		//std::atomic<int>	mActiveWorkers;
		int	mActiveWorkers;  //para implementacion ingenua
		Future<void>	mSignal;
		::core::CriticalSection mCS;

		SimpleBarrierData(size_t nWorkers);
		void addWorkers(size_t nWorkers);
		void set();
		inline FutureData_Base::EWaitResult wait( unsigned int msecs ) const
		{
			return mSignal.wait( msecs );
		}
		inline FutureData_Base::EWaitResult waitAsMThread( unsigned int msecs ) const
		{
			return mSignal.waitAsMThread( msecs );
		}
		inline int getActiveWorkers() const { return mActiveWorkers; }

	};
	class FOUNDATION_API SimpleBarrier : public Barrier
	{
	public:
		SimpleBarrier( size_t nWorkers = 0);
		/**
		* add given number of workers.
		* @return current pending workers
		*/
		inline void addWorkers(size_t nWorkers)
		{
			((SimpleBarrierData*)getData())->addWorkers(nWorkers);
		}
		/**
		* called by each worker to notify barrier was reach
		*/
		inline void set()
		{
			((SimpleBarrierData*)getData())->set();
		}
		inline int getActiveWorkers() const
		{ 
			return ((const SimpleBarrierData*)getData())->getActiveWorkers(); 
		}
	};
}