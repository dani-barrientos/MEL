#include <tasking/utilities.h>
mel::tasking::EEventMTWaitCode
mel::tasking::waitForBarrierMThread( const ::mel::parallelism::Barrier& b,
                                     unsigned int msecs )
{
    using ::mel::tasking::Event_mthread;

    struct _Receiver
    {
        _Receiver() : mEvent( false, false ) {}
        EEventMTWaitCode wait( const ::mel::parallelism::Barrier& barrier,
                               unsigned int msecs )
        {
            EEventMTWaitCode eventresult;
            // spdlog::info("Waiting for event");
            int evId;
            eventresult = mEvent.waitAndDo(
                [this, barrier, &evId]()
                {
                    //   spdlog::debug("waitAndDo was done for Thread
                    //   {}",threadid);
                    evId = barrier.subscribeCallback(
                        std::function<::mel::core::ECallbackResult(
                            const ::mel::parallelism::BarrierData& )>(
                            [this]( const ::mel::parallelism::BarrierData& )
                            {
                                mEvent.set();
                                // spdlog::info("Event was set");
                                return ::mel::core::ECallbackResult::
                                    UNSUBSCRIBE;
                            } ) );
                },
                msecs );
            barrier.unsubscribeCallback( evId );
            return eventresult;
        }

      private:
        mel::tasking::Event_mthread<> mEvent;
    };
    auto receiver = std::make_unique<_Receiver>();
    return receiver->wait( b, msecs );
}