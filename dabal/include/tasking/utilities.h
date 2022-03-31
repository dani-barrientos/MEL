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
#include <utility>
namespace tasking
{
    /**
     * @brief Wait for future ready (valid or error)
     * 
     * @tparam T 
     * @param f future to wait for
     * @param msecs maximum time to wait.
     */

    /**
     * @brief Wait for future ready (valid or error)
     */

/*
    debería lanzar excepcion aqui si hay error? PROS&CONTRAS
    - pro: hace el funcionamiento muy consistente y además compatible con Future<void> (que no tienen un value, y por tanto no puede lanzarse ahi la excepcion)
    - pro: me permite devolver el valor del future directamente, no el wrapper
    - contra: menos flexible?
    template<class T> ::core::WaitResult<T> waitForFutureMThread(  const core::Future<T>& f,unsigned int msecs = ::tasking::Event_mthread::EVENTMT_WAIT_INFINITE)
    {
        using ::tasking::Event_mthread;
        struct _Receiver
        {		
            _Receiver():mEvent(false,false){}
            using futT = core::Future<T>;
            ::core::EWaitError wait( const futT& f,unsigned int msecs)
            {
                Event_mthread::EWaitCode eventResult;
                int evId;
            // spdlog::debug("Waiting for event in Thread {}",threadid);
                eventResult = mEvent.waitAndDo([this,f,&evId]()
                {
                //   spdlog::debug("waitAndDo was done for Thread {}",threadid);
                    evId = f.subscribeCallback(
                    std::function<::core::ECallbackResult( typename futT::ValueType&)>([this](typename futT::ValueType& ) 
                    {
                        mEvent.set();
                     //  spdlog::debug("Event was set for Thread {}");
                        return ::core::ECallbackResult::UNSUBSCRIBE; 
                    }));
                },msecs); 
                f.unsubscribeCallback(evId); //maybe timeout, so callback won't be unsubscribed automatically       
                switch( eventResult )
                {
                case Event_mthread::EVENTMT_WAIT_OK:
                    return ::core::EWaitError::FUTURE_WAIT_OK;
                case Event_mthread::EVENTMT_WAIT_KILL:
                    //event was triggered because a kill signal                    
                    return ::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL;
                    break;
                case Event_mthread::EVENTMT_WAIT_TIMEOUT:
                    return ::core::EWaitError::FUTURE_WAIT_TIMEOUT;
                    break;
                default:
                    return ::core::EWaitError::FUTURE_WAIT_OK; //silent warning
                }			                        
            }
            private:
            ::tasking::Event_mthread mEvent;

        };
        auto receiver = std::make_unique<_Receiver>();
        return ::core::WaitResult<T>(receiver->wait(f,msecs),f);	
    }  
    */
   /**
    * wait for future completion, returning a wapper around the internal vale
    * @throws core::WaitException if some error occured while waiting of the internal future exception if it has any error
    */
   template<class T> ::core::WaitResult<T> waitForFutureMThread(  const core::Future<T>& f,unsigned int msecs = ::tasking::Event_mthread::EVENTMT_WAIT_INFINITE)
    {
        using ::tasking::Event_mthread;
        struct _Receiver
        {		
            _Receiver():mEvent(false,false){}
            using futT = core::Future<T>;
            ::core::EWaitError wait( const futT& f,unsigned int msecs)
            {
                Event_mthread::EWaitCode eventResult;
                int evId;
                eventResult = mEvent.waitAndDo([this,f,&evId]()
                {
                    evId = f.subscribeCallback(
                    std::function<::core::ECallbackResult( typename futT::ValueType&)>([this](typename futT::ValueType& ) 
                    {
                        mEvent.set();
                        return ::core::ECallbackResult::UNSUBSCRIBE; 
                    }));
                },msecs); 
                f.unsubscribeCallback(evId); //maybe timeout, so callback won't be unsubscribed automatically       
                switch( eventResult )
                {
                case Event_mthread::EVENTMT_WAIT_OK:                    
                    return ::core::EWaitError::FUTURE_WAIT_OK;
                case Event_mthread::EVENTMT_WAIT_KILL:
                    //event was triggered because a kill signal                    
                    return ::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL;
                    break;
                case Event_mthread::EVENTMT_WAIT_TIMEOUT:
                    return ::core::EWaitError::FUTURE_WAIT_TIMEOUT;
                    break;
                default:
                    return ::core::EWaitError::FUTURE_WAIT_OK; //silent warning
                }			                        
            }
            private:
            ::tasking::Event_mthread mEvent;

        };
        auto receiver = std::make_unique<_Receiver>();
        ::core::EWaitError waitRes = receiver->wait(f,msecs);
        switch (waitRes)
        {
        case ::core::EWaitError::FUTURE_WAIT_OK:
            if ( f.getValue().isValid())
                return ::core::WaitResult(f);
            else
                std::rethrow_exception(f.getValue().error());
            break;
        case ::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL:
            throw core::WaitException(::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL,"Kill signal received");
            break;
        case ::core::EWaitError::FUTURE_WAIT_TIMEOUT:
            throw core::WaitException(::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL,"Time out exceeded");
            break;
        case ::core::EWaitError::FUTURE_UNKNOWN_ERROR:
            throw core::WaitException(::core::EWaitError::FUTURE_UNKNOWN_ERROR,"Unknown error");
            break;
        default:
            throw core::WaitException(::core::EWaitError::FUTURE_UNKNOWN_ERROR,"Unknown error");
            break;
        }
    }  
    
    DABAL_API ::tasking::Event_mthread::EWaitCode waitForBarrierMThread(const ::parallelism::Barrier& b,unsigned int msecs = ::tasking::Event_mthread::EVENTMT_WAIT_INFINITE );

}