#pragma once
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
#include <tasking/Event_mthread.h>
#include <parallelism/Barrier.h>
namespace tasking
{
    /**
     * @brief Wait for future ready (valid or error)
     * 
     * @tparam T 
     * @param f future to wait for
     * @param msecs maximum time to wait.
     */
    template<class T,class ErrorType = ::core::ErrorInfo> typename core::Future<T,ErrorType>::ValueType waitForFutureMThread(  core::Future<T,ErrorType>& f,unsigned int msecs = ::tasking::Event_mthread::EVENTMT_WAIT_INFINITE)
    {
        using ::tasking::Event_mthread;
        struct _Receiver
        {		
            _Receiver():mEvent(false,false){}
            typename core::Future<T,ErrorType>::ValueType wait( core::Future<T,ErrorType>& f,unsigned int msecs)
            {
                Event_mthread::EWaitCode eventresult;
                int evId;
            // spdlog::debug("Waiting for event in Thread {}",threadid);
                eventresult = mEvent.waitAndDo([this,f,&evId]()
                {
                //   spdlog::debug("waitAndDo was done for Thread {}",threadid);
                    evId = f.subscribeCallback(
                    std::function<::core::ECallbackResult( ::core::FutureValue<T,ErrorType>&)>([this](::core::FutureValue<T,ErrorType>& ) 
                    {
                        mEvent.set();
                     //  spdlog::debug("Event was set for Thread {}");
                        return ::core::ECallbackResult::UNSUBSCRIBE; 
                    }));
                },msecs); 
                f.unsubscribeCallback(evId);
            //  spdlog::debug("Wait was done in Thread {}",threadid);
                switch( eventresult )
                {
                case Event_mthread::EVENTMT_WAIT_KILL:
                    //event was triggered because a kill signal
                    return typename core::Future<T,ErrorType>::ValueType(ErrorType(::core::Future_Base::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL,"Kill signal received"));
                    //result =  ::core::FutureData_Base::EWaitResult::FUTURE_RECEIVED_KILL_SIGNAL;
                    break;
                case Event_mthread::EVENTMT_WAIT_TIMEOUT:
                    //result = ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_TIMEOUT;
                    return typename core::Future<T,ErrorType>::ValueType(ErrorType(::core::Future_Base::EWaitError::FUTURE_WAIT_TIMEOUT,"Time out exceeded"));
                    break;
                default:
                    return f.getValue();
                    break;
                }			        
            }
            private:
            ::tasking::Event_mthread mEvent;

        };
        auto receiver = std::make_unique<_Receiver>();
        return receiver->wait(f,msecs);	
    }  
    
    DABAL_API ::tasking::Event_mthread::EWaitCode waitForBarrierMThread(const ::parallelism::Barrier& b,unsigned int msecs = ::tasking::Event_mthread::EVENTMT_WAIT_INFINITE );

}