#if defined(MEL_LINUX) || defined (MEL_MACOSX) || defined(MEL_ANDROID) || defined(MEL_IOS)
#include <core/Event.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/errno.h>
namespace mel
{
namespace core {


Event::Event(bool autoRelease,bool signaled):
	mAutoRelease(autoRelease),
	mSignaled(false) {
	pthread_mutex_init( &_mutex, NULL );
	pthread_cond_init( &_cond, NULL );
	if (signaled) {
		set();
		wait();
		wait();
	}
}

Event::~Event() {
	pthread_cond_destroy( &_cond );
	pthread_mutex_destroy( &_mutex );
}

void Event::set() {
	pthread_mutex_lock( &_mutex );
	mSignaled = true;
	pthread_cond_broadcast( &_cond );
	pthread_mutex_unlock( &_mutex );
}

/*
void Event::setOne() {
	pthread_mutex_lock( &_mutex );
	if (!mSignaled) {
		mSignaled = true;
		pthread_cond_signal(&_cond );
	}
	pthread_mutex_unlock( &_mutex );
	
}
*/

Event::EWaitCode Event::wait( unsigned int msegs) const 
{
    EWaitCode result = EVENT_WAIT_OK;
    pthread_mutex_lock( &_mutex );
    if ( msegs == - 1)
        while ( mSignaled == false ) {
            pthread_cond_wait( &_cond, &_mutex );
        }
    else 
    {
        struct timeval now;
        struct timespec ts;
        gettimeofday(&now, NULL);
        ts.tv_sec = now.tv_sec+(msegs/1000);
        
        ts.tv_nsec = now.tv_usec*1000UL + (msegs%1000)*1000UL;
        
        int retcode = pthread_cond_timedwait( &_cond, &_mutex,&ts );
        switch ( retcode )
        {
            case EINVAL: result = EVENT_WAIT_ERROR;break;
            case ETIMEDOUT: result = EVENT_WAIT_TIMEOUT;break;
            default: result = EVENT_WAIT_OK;
        }
    }
    // if we're an auto-reset event, auto reset
    mSignaled = !mAutoRelease;
    pthread_mutex_unlock( &_mutex );
    return result;
}

void Event::reset() {
	pthread_mutex_lock( &_mutex );
	mSignaled = false;
	pthread_mutex_unlock( &_mutex );
}

//end namespace
}
}
#endif