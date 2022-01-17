/**
 * @file utilities.h
 * @author Daniel Barrientos (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-01-16
 * 
 * @copyright Copyright (c) 2022
 *  Utilities on task, because lack of a better place...Some funcions are intended to be put in a custom file/class..
 * 
 */
#include <core/Future.h>
#include <memory>
#include <core/Event_mthread.h>
namespace tasking
{
    template<class T> ::core::FutureData_Base::EWaitResult waitForFutureMThread( const core::Future<T>& f,unsigned int msecs = ::core::Event_mthread::EVENTMT_WAIT_INFINITE)
{
	using ::core::Event_mthread;
	using ::core::FutureData_Base;
    using ::core::FutureData;
	struct _Receiver
	{		
		_Receiver():mEvent(false,false){}
		FutureData_Base::EWaitResult wait(const core::Future<T>& f,unsigned int msecs)
		{
            FutureData_Base::EWaitResult result;            
            Event_mthread::EWaitCode eventresult;
           // spdlog::debug("Waiting for event in Thread {}",threadid);
            eventresult = mEvent.waitAndDo([this,f]()
            {
             //   spdlog::debug("waitAndDo was done for Thread {}",threadid);
                f.subscribeCallback(
                std::function<::core::ECallbackResult( const FutureData<T>&)>([this](const FutureData<T>& ) 
                {
                    mEvent.set();
                 //   spdlog::debug("Event was set for Thread {}",threadid);
                    return ::core::ECallbackResult::UNSUBSCRIBE; 
                }));
            },msecs); 
          //  spdlog::debug("Wait was done in Thread {}",threadid);
            switch( eventresult )
            {
            case ::core::Event_mthread::EVENTMT_WAIT_KILL:
                //event was triggered because a kill signal
                result = ::core::FutureData_Base::EWaitResult::FUTURE_RECEIVED_KILL_SIGNAL;
                break;
            case Event_mthread::EVENTMT_WAIT_TIMEOUT:
                result = ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_TIMEOUT;
                break;
            default:
                result = ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_OK;
                break;
            }			
			return result;	
	
		}
		private:
		::core::Event_mthread mEvent;

	};
	auto receiver = make_unique<_Receiver>();
	return receiver->wait(f,msecs);	
}

template<> ::core::FutureData_Base::EWaitResult waitForFutureMThread<void>( const core::Future<void>& f,unsigned int msecs)
{
	using ::core::Event_mthread;
	using ::core::FutureData_Base;
    using ::core::FutureData;
	struct _Receiver
	{		
		_Receiver():mEvent(false,false){}
		FutureData_Base::EWaitResult wait(const core::Future<void>& f,unsigned int msecs)
		{
            FutureData_Base::EWaitResult result;            
            Event_mthread::EWaitCode eventresult;
           // spdlog::debug("Waiting for event in Thread {}",threadid);
            eventresult = mEvent.waitAndDo([this,f]()
            {
             //   spdlog::debug("waitAndDo was done for Thread {}",threadid);
                f.subscribeCallback(
                std::function<::core::ECallbackResult(const FutureData<void>&)>([this]( const FutureData<void>&) 
                {
                    mEvent.set();
                 //   spdlog::debug("Event was set for Thread {}",threadid);
                    return ::core::ECallbackResult::UNSUBSCRIBE; 
                }));
            },msecs); 
          //  spdlog::debug("Wait was done in Thread {}",threadid);
            switch( eventresult )
            {
            case ::core::Event_mthread::EVENTMT_WAIT_KILL:
                //event was triggered because a kill signal
                result = ::core::FutureData_Base::EWaitResult::FUTURE_RECEIVED_KILL_SIGNAL;
                break;
            case Event_mthread::EVENTMT_WAIT_TIMEOUT:
                result = ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_TIMEOUT;
                break;
            default:
                result = ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_OK;
                break;
            }			
			return result;	
	
		}
		private:
		::core::Event_mthread mEvent;

	};
	auto receiver = make_unique<_Receiver>();
	return receiver->wait(f,msecs);	
}
}