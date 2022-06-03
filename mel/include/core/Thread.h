#pragma once

#include <core/Event.h>
#include <core/Future.h>
#include <core/ThreadDefs.h>
#include <memory>
#include <parallelism/Barrier.h>
#include <thread>

namespace mel
{
    /**
     * @brief base functionalities
     */
    namespace core
    {
        /**
         * @brief Get the number of logical processors
         */
        MEL_API unsigned int getNumProcessors();
        MEL_API uint64_t getProcessAffinity();
        //! Set affinity for current thread.
        MEL_API bool setAffinity( uint64_t );
        using std::string;

        /**
         * @class Thread
         * @brief Wrapper around std::thread to prive more abstractions
         * @warning Some of these features may not be present on certain
         * platforms, or may have specific requirements.
         */
        class MEL_API Thread
        {

          public:
            //@brief policy for yield() function
            enum YieldPolicy
            {
                YP_ANY_THREAD_ANY_PROCESSOR = 0,
                YP_ANY_THREAD_SAME_PROCESSOR
            };

            /**
             * @brief Starts the main thread routine.
             * @param[in] f funtion with signature void f()
             */
            template <class F> Thread( F&& f );
            // not a thread
            Thread();
            /**
             * @brief Thread is joined on destruction
             */
            virtual ~Thread();

            /**
             * Changes thread priority.
             * May not be available in some platforms.
             * @param tp the new priority to be set.
             */
            void setPriority( EThreadPriority tp );
            /**
             * Query thread's current priority.
             * @return the priority of the thread
             */
            inline EThreadPriority getPriority() const;

            /**
             * Forces the caller to wait for thread completion.
             * Calling this method will cause the calling thread to be wait
             * until Thread:run finishes or the timeout expires.
             * @param millis maximum milliseconds to wait for the thread.
             * @return true if the thread finished. false if timeout
             */
            bool
            join( unsigned int millis = 0xFFFFFFFF /*TODO mac: INFINITE*/ );

            /**
             * Forces the calling thread to sleep.
             * Calling this method ensures the OS will schedule some CPU for any
             * pending processes and threads that need attention.<br> NOTE: on
             * some platforms, calling sleep(0) may cause the call to be
             * completely ignored.
             * @param millis the number of milliseconds to sleep for. The actual
             * sleep time may depend on each platform, but you can expect a
             * granularity not finer than 10ms, meaning sleep(1) will make the
             * thread sleep for almost the same time as sleep(10).
             */
            static void sleep( const unsigned int millis );
            /**
             * Forces the calling thread to yield execution.
             * This may have slight different effects depending on the platform,
             * but theoretically the execution is yielded only to threads in the
             * same process as the caller.<br> Calling this method may not force
             * the OS to schedule any CPU time for any pending processes
             * different that the caller.
             */
            static void yield( YieldPolicy yp = YP_ANY_THREAD_SAME_PROCESSOR );

            /**
             * return handle for this thread
             */
            inline std::thread::id getThreadId() const;
            /**
             * Get thread state
             * @return the thread's current state
             */
            // inline EThreadState getState() const;
            /**
             * returns if a terminate request is done (mEnd == true)
             */
            // inline bool getTerminateRequest() const;

            uint64_t getAffinity() const;
            /**
             * @todo no est� protegido frente a llamada con el hilo iniciandose
             */
            bool setAffinity( uint64_t );
            /**
             * return minimun time(msecs) the system will wait with accuracy.
             * it depends on underlying OS and hardware, but traditionally it's
             * about 10-15 msecs
             * @todo for the moment we will return a fixed contant depending on
             * platform
             */
            constexpr static unsigned getMinimunSleepTime()
            {
                //@todo por ahora pongo tiempo fijo "t�pico" para que se pueda
                // usar y ya trataremos de que sea autom�tico o al menos m�s
                // flexible
                constexpr unsigned MINIMUM_SLEEP = 10;
                return MINIMUM_SLEEP;
            }
            /**
             * Forces thread termination
             * Use with extreme caution; this method exists but it should
             * rarely be used; it's always safer to invoke terminatRequest and
             * wait for the Thread to terminate "naturally", instead of forcing
             * it to quite.
             * @param exitCode the exit code for the terminated thread.
             */
            void terminate( unsigned int exitCode = 0 );

          private:
            Thread( const char* name );
            enum class EJoinResult
            {
                JOINED_NONE,
                JOINED_OK,
                JOINED_ERROR
            } mJoinResult;
#if defined( MEL_LINUX ) || defined( MEL_MACOSX ) || defined( MEL_ANDROID ) || \
    defined( MEL_IOS )
            int mPriorityMin;
            int mPriorityMax;
#endif
            uint64_t mAffinity = 0; // affinity to set on start. if 0, is
                                    // ignored
            unsigned int mExitCode;
            EThreadPriority mPriority = EThreadPriority::TP_NONE;

            std::thread mThread;
            void _initialize();
        };
        template <class F>
        Thread::Thread( F&& f ) : mThread( std::forward<F>( f ) )
        {
            _initialize();
        }
        std::thread::id Thread::getThreadId() const { return mThread.get_id(); }

        EThreadPriority Thread::getPriority() const { return mPriority; }

        /**
         * @brief Wait for a Future from a Thread
         */
        /*template<class T> ::mel::core::WaitResult<T> waitForFutureThread(
        const mel::core::Future<T>& f,unsigned int msecs =
        ::mel::core::Event::EVENT_WAIT_INFINITE)
        {
                using ::mel::core::Event;
                struct _Receiver
                {
                        _Receiver():mEvent(false,false){}
                        using futT = mel::core::Future<T>;
                        mel::core::EWaitError wait( const futT& f,unsigned int
        msecs)
                        {
                                Event::EWaitCode eventresult;
                                int evId = f.subscribeCallback([this](typename
        futT::ValueType& )
                                        {
                                                mEvent.set();
                                        });
                                eventresult = mEvent.wait(msecs);
                                f.unsubscribeCallback(evId);
                                switch( eventresult )
                                {
                                case ::mel::core::Event::EVENT_WAIT_OK:
                                        return
        ::mel::core::EWaitError::FUTURE_WAIT_OK; break; case
        ::mel::core::Event::EVENT_WAIT_TIMEOUT: return
        ::mel::core::EWaitError::FUTURE_WAIT_TIMEOUT; break; case
        ::mel::core::Event::EVENT_WAIT_ERROR: return
        ::mel::core::EWaitError::FUTURE_UNKNOWN_ERROR; break; default: return
        ::mel::core::EWaitError::FUTURE_WAIT_OK; //silent warnin
                                }
                        }
                        private:
                                mel::core::Event mEvent;
                };
                auto receiver = std::make_unique<_Receiver>();
                mel::core::EWaitError waitRes = receiver->wait(f,msecs);
                switch (waitRes)
                {
                case ::mel::core::EWaitError::FUTURE_WAIT_OK:
                        if ( f.getValue().isValid())
                                return ::mel::core::WaitResult(f);
                        else
                                std::rethrow_exception(f.getValue().error());
                        break;
                case ::mel::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL:
                        throw
        mel::core::WaitException(mel::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL,"Kill
        signal received"); break; case
        ::mel::core::EWaitError::FUTURE_WAIT_TIMEOUT: throw
        mel::core::WaitException(mel::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL,"Time
        out exceeded"); break; case
        ::mel::core::EWaitError::FUTURE_UNKNOWN_ERROR: throw
        mel::core::WaitException(mel::core::EWaitError::FUTURE_UNKNOWN_ERROR,"Unknown
        error"); break; default: throw
        mel::core::WaitException(mel::core::EWaitError::FUTURE_UNKNOWN_ERROR,"Unknown
        error"); break;
                }
                */
        template <class ErrorType = mel::core::WaitErrorAsException, class T>
        ::mel::core::WaitResult<T> waitForFutureThread(
            const mel::core::Future<T>& f,
            unsigned int msecs = ::mel::core::Event::
                EVENT_WAIT_INFINITE ) noexcept( std::
                                                    is_same<
                                                        ErrorType,
                                                        ::mel::core::
                                                            WaitErrorNoException>::
                                                        value )
        {
            constexpr bool NotuseException =
                std::is_same<ErrorType,
                             ::mel::core::WaitErrorNoException>::value;
            constexpr bool UseException =
                std::is_same<ErrorType,
                             ::mel::core::WaitErrorAsException>::value;
            static_assert( NotuseException || UseException,
                           "WaitForFutureMThread must specify either "
                           "WaitErrorNoException or WaitErrorAsException" );
            using ::mel::core::Event;
            struct _Receiver
            {
                _Receiver() : mEvent( false, false ) {}
                using futT = mel::core::Future<T>;
                mel::core::EWaitError wait( const futT& f, unsigned int msecs )
                {
                    Event::EWaitCode eventresult;
                    int evId = f.subscribeCallback(
                        [this]( typename futT::ValueType& ) { mEvent.set(); } );
                    eventresult = mEvent.wait( msecs );
                    f.unsubscribeCallback( evId );
                    switch ( eventresult )
                    {
                    case ::mel::core::Event::EVENT_WAIT_OK:
                        return ::mel::core::EWaitError::FUTURE_WAIT_OK;
                        break;
                    case ::mel::core::Event::EVENT_WAIT_TIMEOUT:
                        return ::mel::core::EWaitError::FUTURE_WAIT_TIMEOUT;
                        break;
                    case ::mel::core::Event::EVENT_WAIT_ERROR:
                        return ::mel::core::EWaitError::FUTURE_UNKNOWN_ERROR;
                        break;
                    default:
                        return ::mel::core::EWaitError::
                            FUTURE_WAIT_OK; // silent warnin
                    }
                }

              private:
                mel::core::Event mEvent;
            };
            ::mel::core::WaitResult result( f );
            auto receiver = std::make_unique<_Receiver>();
            mel::core::EWaitError waitRes = receiver->wait( f, msecs );
            if constexpr ( NotuseException )
            {
                switch ( waitRes )
                {
                case ::mel::core::EWaitError::FUTURE_WAIT_OK:
                    break;
                case ::mel::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL:
                    result.setError(
                        std::make_exception_ptr( mel::core::WaitException(
                            mel::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL,
                            "Kill signal received" ) ) );
                    break;
                case ::mel::core::EWaitError::FUTURE_WAIT_TIMEOUT:
                    result.setError(
                        std::make_exception_ptr( mel::core::WaitException(
                            mel::core::EWaitError::FUTURE_WAIT_TIMEOUT,
                            "Time out exceeded" ) ) );
                    break;
                case ::mel::core::EWaitError::FUTURE_UNKNOWN_ERROR:
                    result.setError(
                        std::make_exception_ptr( mel::core::WaitException(
                            mel::core::EWaitError::FUTURE_UNKNOWN_ERROR,
                            "Unknown error" ) ) );
                    break;
                default:
                    result.setError(
                        std::make_exception_ptr( mel::core::WaitException(
                            mel::core::EWaitError::FUTURE_UNKNOWN_ERROR,
                            "Unknown error" ) ) );
                    break;
                }
            }
            else // verion throwing exception (UseExceptoion == true)
            {
                switch ( waitRes )
                {
                case ::mel::core::EWaitError::FUTURE_WAIT_OK:
                    if ( f.getValue().isValid() )
                        return ::mel::core::WaitResult( f );
                    else
                        std::rethrow_exception( f.getValue().error() );
                    break;
                case ::mel::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL:
                    throw mel::core::WaitException(
                        mel::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL,
                        "Kill signal received" );
                    break;
                case ::mel::core::EWaitError::FUTURE_WAIT_TIMEOUT:
                    throw mel::core::WaitException(
                        mel::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL,
                        "Time out exceeded" );
                    break;
                case ::mel::core::EWaitError::FUTURE_UNKNOWN_ERROR:
                    throw mel::core::WaitException(
                        mel::core::EWaitError::FUTURE_UNKNOWN_ERROR,
                        "Unknown error" );
                    break;
                default:
                    throw mel::core::WaitException(
                        mel::core::EWaitError::FUTURE_UNKNOWN_ERROR,
                        "Unknown error" );
                    break;
                }
            }
            return result;
        }

        /**
         * @brief Wait for a \ref ::mel::parallelism::Barrier "barrier" to
         * activated in the context of a thread
         */
        MEL_API ::mel::core::Event::EWaitCode
        waitForBarrierThread( const ::mel::parallelism::Barrier& b,
                              unsigned int msecs = Event::EVENT_WAIT_INFINITE );
    } // namespace core
} // namespace mel
