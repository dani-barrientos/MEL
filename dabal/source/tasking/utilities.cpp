#include <tasking/utilities.h>
::tasking::EEventMTWaitCode tasking::waitForBarrierMThread(const ::parallelism::Barrier& b,unsigned int msecs)
{
	using ::tasking::Event_mthread;

	struct _Receiver
	{		
		_Receiver():mEvent(false,false){}
		EEventMTWaitCode wait(const ::parallelism::Barrier& barrier,unsigned int msecs)
		{
			EEventMTWaitCode eventresult;
		 	//spdlog::info("Waiting for event");
			int evId;
			eventresult = mEvent.waitAndDo([this,barrier,&evId]()
			{
			//   spdlog::debug("waitAndDo was done for Thread {}",threadid);
				evId = barrier.subscribeCallback(
				std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([this](const ::parallelism::BarrierData& ) 
				{
					 mEvent.set();
				    //spdlog::info("Event was set");
					return ::core::ECallbackResult::UNSUBSCRIBE; 
				}));
			},msecs); 
			barrier.unsubscribeCallback(evId);
			return eventresult;
		}
		private:
		::tasking::Event_mthread<> mEvent;

	};
	auto receiver = std::make_unique<_Receiver>();
	return receiver->wait(b,msecs);	
}