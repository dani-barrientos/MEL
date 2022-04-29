#pragma once
/*
 * SPDX-FileCopyrightText: 2005,2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
/**
 * @file utilities.h
 * @brief Utilities on tasking, because lack of a better place...Some funcions are intended to be put in a custom file/class..
 * 
 */
#include <core/Future.h>
#include <memory>
#include <tasking/Event_mthread.h>
#include <parallelism/Barrier.h>
#include <utility>
namespace mel
{
    namespace tasking
    {
        
       /*
        template<class T> ::mel::core::WaitResult<T> waitForFutureMThread(  const mel::core::Future<T>& f,unsigned int msecs = EVENTMT_WAIT_INFINITE)
        {
            using ::mel::tasking::Event_mthread;
            struct _Receiver
            {		
                _Receiver():mEvent(false,false){}
                using futT = mel::core::Future<T>;
                ::mel::core::EWaitError wait( const futT& f,unsigned int msecs)
                {
                    EEventMTWaitCode eventResult;
                    int evId;
                    eventResult = mEvent.waitAndDo([this,f,&evId]()
                    {
                        evId = f.subscribeCallback(
                            [this](typename futT::ValueType& ) 
                            {
                                mEvent.set();                        
                            }
                        );
                    },msecs); 
                    f.unsubscribeCallback(evId); //maybe timeout, so callback won't be unsubscribed automatically       
                    switch( eventResult )
                    {
                    case EEventMTWaitCode::EVENTMT_WAIT_OK:                    
                        return ::mel::core::EWaitError::FUTURE_WAIT_OK;
                    case EEventMTWaitCode::EVENTMT_WAIT_KILL:
                        //event was triggered because a kill signal                    
                        return ::mel::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL;
                        break;
                    case EEventMTWaitCode::EVENTMT_WAIT_TIMEOUT:
                        return ::mel::core::EWaitError::FUTURE_WAIT_TIMEOUT;
                        break;
                    default:
                        return ::mel::core::EWaitError::FUTURE_WAIT_OK; //silent warning
                    }			                        
                }
                private:
                ::mel::tasking::Event_mthread<> mEvent;

            };

            auto receiver = std::make_unique<_Receiver>();
            ::mel::core::EWaitError waitRes = receiver->wait(f,msecs);
            switch (waitRes)
            {
            case ::mel::core::EWaitError::FUTURE_WAIT_OK:
                if ( f.getValue().isValid())
                    return ::mel::core::WaitResult(f);
                else
                    std::rethrow_exception(f.getValue().error());
                break;
            case ::mel::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL:
                throw mel::core::WaitException(mel::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL,"Kill signal received");
                break;
            case ::mel::core::EWaitError::FUTURE_WAIT_TIMEOUT:
                throw mel::core::WaitException(mel::core::EWaitError::FUTURE_WAIT_TIMEOUT,"Time out exceeded");
                break;
            case ::mel::core::EWaitError::FUTURE_UNKNOWN_ERROR:
                throw mel::core::WaitException(mel::core::EWaitError::FUTURE_UNKNOWN_ERROR,"Unknown error");
                break;
            default:
                throw mel::core::WaitException(mel::core::EWaitError::FUTURE_UNKNOWN_ERROR,"Unknown error");
                break;
            }
        }  
*/
        /**
        * @brief Waits for future completion, returning a wapper around the internal vale
        * @param ErrorType specify how error is managed, by throwing an exception (if used mel::core::WaitErrorAsException) or not (uf used mel::core::WaitErrorNoException)
        * @throws if ErrorType == mel::core::WaitErrorAsException, throws mel::core::WaitException if some error occured while waiting of the internal future exception if it has any error
        */
        template<class ErrorType = mel::core::WaitErrorAsException,class T> ::mel::core::WaitResult<T> waitForFutureMThread(  const mel::core::Future<T>& f,unsigned int msecs = EVENTMT_WAIT_INFINITE) noexcept(std::is_same<ErrorType,::mel::core::WaitErrorNoException>::value)
        {
            constexpr bool NotuseException = std::is_same<ErrorType,::mel::core::WaitErrorNoException>::value;
            constexpr bool UseException = std::is_same<ErrorType,::mel::core::WaitErrorAsException>::value;
            static_assert(NotuseException || UseException,"WaitForFutureMThread must specify either WaitErrorNoException or WaitErrorAsException");
            using ::mel::tasking::Event_mthread;
            struct _Receiver
            {		
                _Receiver():mEvent(false,false){}
                using futT = mel::core::Future<T>;
                ::mel::core::EWaitError wait( const futT& f,unsigned int msecs)
                {
                    EEventMTWaitCode eventResult;
                    int evId;
                    eventResult = mEvent.waitAndDo([this,f,&evId]()
                    {
                        evId = f.subscribeCallback(
                            [this](typename futT::ValueType& ) 
                            {
                                mEvent.set();                        
                            }
                        );
                    },msecs); 
                    f.unsubscribeCallback(evId); //maybe timeout, so callback won't be unsubscribed automatically       
                    switch( eventResult )
                    {
                    case EEventMTWaitCode::EVENTMT_WAIT_OK:                    
                        return ::mel::core::EWaitError::FUTURE_WAIT_OK;
                    case EEventMTWaitCode::EVENTMT_WAIT_KILL:
                        //event was triggered because a kill signal                    
                        return ::mel::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL;
                        break;
                    case EEventMTWaitCode::EVENTMT_WAIT_TIMEOUT:
                        return ::mel::core::EWaitError::FUTURE_WAIT_TIMEOUT;
                        break;
                    default:
                        return ::mel::core::EWaitError::FUTURE_WAIT_OK; //silent warning
                    }			                        
                }
                private:
                ::mel::tasking::Event_mthread<> mEvent;

            };

            ::mel::core::WaitResult result(f);
            auto receiver = std::make_unique<_Receiver>();            
            ::mel::core::EWaitError waitRes = receiver->wait(f,msecs);
            if constexpr (NotuseException)
            {
                switch (waitRes)
                {
                case ::mel::core::EWaitError::FUTURE_WAIT_OK:
                    break;
                case ::mel::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL:
                    result.setError( std::make_exception_ptr(mel::core::WaitException(mel::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL,"Kill signal received")));
                    break;
                case ::mel::core::EWaitError::FUTURE_WAIT_TIMEOUT:
                    result.setError( std::make_exception_ptr(mel::core::WaitException(mel::core::EWaitError::FUTURE_WAIT_TIMEOUT,"Time out exceeded")));
                    break;
                case ::mel::core::EWaitError::FUTURE_UNKNOWN_ERROR:
                    result.setError( std::make_exception_ptr(mel::core::WaitException(mel::core::EWaitError::FUTURE_UNKNOWN_ERROR,"Unknown error")));
                    break;
                default:
                    result.setError( std::make_exception_ptr(mel::core::WaitException(mel::core::EWaitError::FUTURE_UNKNOWN_ERROR,"Unknown error")));
                    break;
                }
            }else  //verion throwing exception (UseExceptoion == true)
            {
                switch (waitRes)
                {
                case ::mel::core::EWaitError::FUTURE_WAIT_OK:
                    if ( f.getValue().isValid())
                        return ::mel::core::WaitResult(f);
                    else
                        std::rethrow_exception(f.getValue().error());
                    break;
                case ::mel::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL:
                    throw mel::core::WaitException(mel::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL,"Kill signal received");
                    break;
                case ::mel::core::EWaitError::FUTURE_WAIT_TIMEOUT:
                    throw mel::core::WaitException(mel::core::EWaitError::FUTURE_WAIT_TIMEOUT,"Time out exceeded");
                    break;
                case ::mel::core::EWaitError::FUTURE_UNKNOWN_ERROR:
                    throw mel::core::WaitException(mel::core::EWaitError::FUTURE_UNKNOWN_ERROR,"Unknown error");
                    break;
                default:
                    throw mel::core::WaitException(mel::core::EWaitError::FUTURE_UNKNOWN_ERROR,"Unknown error");
                    break;
                }
            }
            return result;
        }  

        /**
         * @brief Wait for a \ref ::mel::parallelism::Barrier "barrier" to activated in the context of a \ref ::mel::tasking::Process "microthread"
         */
        MEL_API ::mel::tasking::EEventMTWaitCode waitForBarrierMThread(const ::mel::parallelism::Barrier& b,unsigned int msecs = ::mel::tasking::EVENTMT_WAIT_INFINITE );

    }
}