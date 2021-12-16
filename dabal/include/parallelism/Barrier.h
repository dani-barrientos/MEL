#pragma once
#include <core/IRefCount.h>
#include <core/SmartPtr.h>
#include <core/Future.h>
using ::core::FutureData_Base;

namespace parallelism
{
	using ::core::SmartPtr;
	using ::core::IRefCount;
	using ::core::Future;
	class Barrier;
	class BarrierData : public IRefCount
	{
		friend class ::parallelism::Barrier;
	private:
		virtual FutureData_Base::EWaitResult wait( unsigned int msecs ) const = 0;
		virtual FutureData_Base::EWaitResult waitAsMThread( unsigned int msecs ) const = 0;
	protected:

	};

	class FOUNDATION_API Barrier
	{
	public:
		Barrier( const Barrier& o2){ mData = o2.mData;}
		FutureData_Base::EWaitResult wait( unsigned int msecs = ::core::Event::EVENT_WAIT_INFINITE ) const
		{
			return mData->wait( msecs );
		}
		FutureData_Base::EWaitResult waitAsMThread( unsigned int msecs = ::core::Event_mthread::EVENTMT_WAIT_INFINITE) const
		{
			return mData->waitAsMThread( msecs );
		}
	private:
		SmartPtr<BarrierData>	mData;
	protected:
		Barrier( BarrierData* data );
		inline BarrierData* getData(){ return mData;}
		inline const BarrierData* getData() const { return mData; }
	};
}