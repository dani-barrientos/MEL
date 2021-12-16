#include <parallelism/SimpleBarrier.h>
using namespace parallelism;

SimpleBarrierData::SimpleBarrierData(size_t nWorkers) :mActiveWorkers((int)nWorkers)
{
	if (nWorkers == 0)
	{
		::core::Lock lck(mCS);
		mSignal.setValue();
	}
}
void SimpleBarrierData::set()
{
	::core::Lock lck(mCS);
	if ( --mActiveWorkers == 0 )  //use memory_order_seq_cst
	//if ( (atomic_fetch_sub_explicit(&mActiveWorkers,1,std::memory_order_relaxed) - 1) == 0)
		mSignal.setValue();
	//if ( ::core::atomicDecrement(&mActiveWorkers) == 0 )
	//	mSignal.setValue();
}
void SimpleBarrierData::addWorkers(size_t nWorkers)
{
	::core::Lock lck(mCS);
	mActiveWorkers += nWorkers;
	if (mSignal.getValid()) //already signaled?
		mSignal = Future<void>(); //reset signal
}
SimpleBarrier::SimpleBarrier( size_t nWorkers ):Barrier( new SimpleBarrierData( nWorkers ) )
{
}
