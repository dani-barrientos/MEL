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
/*    eto de devolver la referencia no es correcto, ya que el future puede morir (y de hecho me pasa cuando encadeno llamadas)
    ADEMÁS ES MUY GRAVE QUE estoy modificndo aquí el propio future!!!

    POSIBILIDADES:
     - VERSION TEMPLATICA: si la sobrecarga es rvaluereference, devolver un valuetype nuevo per ohecho el move
     - si es una referencia puiedo devolver como ahora->pero no puedo modificar el future. 
    En cualquier caso no se puede hacer asi porque lo de modifica rel propio future es INCORRECTO
    VOY A TENER QUE DEJARLO COMO ANTIGUAMENTE
    */
   /*
    template<class T,class ErrorType = ::core::ErrorInfo> typename core::Future<T,ErrorType>::ValueType& waitForFutureMThread(  core::Future<T,ErrorType> f,unsigned int msecs = ::tasking::Event_mthread::EVENTMT_WAIT_INFINITE)
    {
        using ::tasking::Event_mthread;
        struct _Receiver
        {		
            _Receiver():mEvent(false,false){}
            using futT = core::Future<T,ErrorType>;
            typename core::Future<T,ErrorType>::ValueType& wait( futT& f,unsigned int msecs)
            {
                Event_mthread::EWaitCode eventresult;
                int evId;
            // spdlog::debug("Waiting for event in Thread {}",threadid);
                eventresult = mEvent.waitAndDo([this,f,&evId]()
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
                f.unsubscribeCallback(evId);
            //  spdlog::debug("Wait was done in Thread {}",threadid);
                switch( eventresult )
                {
                case Event_mthread::EVENTMT_WAIT_KILL:
                    //event was triggered because a kill signal                    
                    f.setError(ErrorType(::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL,"Kill signal received"));
                    //return typename core::Future<T,ErrorType>::ValueType(ErrorType(::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL,"Kill signal received"));
                    break;
                case Event_mthread::EVENTMT_WAIT_TIMEOUT:
                    f.setError(ErrorType(::core::EWaitError::FUTURE_WAIT_TIMEOUT,"Time out exceeded"));
                    //return typename core::Future<T,ErrorType>::ValueType(ErrorType(::core::EWaitError::FUTURE_WAIT_TIMEOUT,"Time out exceeded"));
                    break;
                }			        
                return f.getValue();
            }
            private:
            ::tasking::Event_mthread mEvent;

        };
        auto receiver = std::make_unique<_Receiver>();
        return receiver->wait(f,msecs);	
    }  
    */
   
    /**
     * @brief Wait for future ready (valid or error)
     */
    template<class T,class ErrorType = ::core::ErrorInfo> ::core::WaitResult<T,ErrorType> waitForFutureMThread(  const core::Future<T,ErrorType>& f,unsigned int msecs = ::tasking::Event_mthread::EVENTMT_WAIT_INFINITE)
    {
        using ::tasking::Event_mthread;
        struct _Receiver
        {		
            _Receiver():mEvent(false,false){}
            using futT = core::Future<T,ErrorType>;
            ::core::EWaitError wait( const futT& f,unsigned int msecs)
            {
                Event_mthread::EWaitCode eventResult;
                int evId;
            // spdlog::debug("Waiting for event in Thread {}",threadid);
                eventResult = mEvent.waitAndDo([this,f,&evId]()
                {
                //   spdlog::debug("waitAndDo was done for Thread {}",threadid);
                    evId = f.subscribeCallback(
                    std::function<::core::ECallbackResult( const typename futT::ValueType&)>([this](const typename futT::ValueType& ) 
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
        return ::core::WaitResult<T,ErrorType>(receiver->wait(f,msecs),f);	
    }  
    
    DABAL_API ::tasking::Event_mthread::EWaitCode waitForBarrierMThread(const ::parallelism::Barrier& b,unsigned int msecs = ::tasking::Event_mthread::EVENTMT_WAIT_INFINITE );

}