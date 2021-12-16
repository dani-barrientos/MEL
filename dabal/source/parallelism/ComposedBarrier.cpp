#include <parallelism/ComposedBarrier.h>
using namespace parallelism;


ComposedBarrier::ComposedBarrier( ):Barrier( new ComposedBarrierData( ) ){}
FutureData_Base::EWaitResult ComposedBarrierData::wait( unsigned int msecs ) const
{
	FutureData_Base::EWaitResult result = FutureData_Base::FUTURE_WAIT_OK;
	//AND
	for( BarrierList::const_iterator i = mBarriers.begin(),j = mBarriers.end(); i != j; ++i )
	{
			result  = i->wait();
			if ( result != FutureData_Base::FUTURE_WAIT_OK )
				return result;
	}
	return result;
}
FutureData_Base::EWaitResult ComposedBarrierData::waitAsMThread( unsigned int msecs ) const
{
	FutureData_Base::EWaitResult result = FutureData_Base::FUTURE_WAIT_OK;
	//AND
	for( BarrierList::const_iterator i = mBarriers.begin(),j = mBarriers.end(); i != j; ++i )
	{
		result  = i->waitAsMThread();
		if ( result != FutureData_Base::FUTURE_WAIT_OK )
			return result;
	}
	return result;
}