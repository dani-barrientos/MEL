#if defined( _WINDOWS )
#include <assert.h>
#include <core/Event.h>
namespace mel
{
    namespace core
    {

        Event::Event( bool autoRelease, bool signaled )
        {
            mEvent = CreateEvent( NULL, autoRelease ? FALSE : TRUE,
                                  signaled ? TRUE : FALSE, NULL );
            assert( mEvent && "Unable to create event handle!" );
        }

        Event::~Event() { CloseHandle( mEvent ); }

        void Event::set() { SetEvent( mEvent ); }

        Event::EWaitCode Event::wait( unsigned int msecs ) const
        {
            EWaitCode result;
            DWORD code = WaitForSingleObject( mEvent, msecs );
            switch ( code )
            {
            case WAIT_TIMEOUT:
                result = EVENT_WAIT_TIMEOUT;
                break;
            case WAIT_OBJECT_0:
                result = EVENT_WAIT_OK;
                break;
            default:
                result = EVENT_WAIT_ERROR;
                break;
            }
            return result;
        }

        void Event::reset() { ResetEvent( mEvent ); }

        // end namespace
    } // namespace core
} // namespace mel
#endif