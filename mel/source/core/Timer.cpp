#include <core/Timer.h>
using mel::core::Timer;

Timer::Timer() : mStartTime( 0 ), mState( ACTIVE ), mMsActive( 0 ) { reset(); }

Timer::~Timer()
{
    // OK. Do nothing
}

void Timer::reset()
{
#ifdef _WINDOWS
    QueryPerformanceFrequency( (LARGE_INTEGER*)&mFrequency );
    QueryPerformanceCounter( (LARGE_INTEGER*)&mReference );
#elif defined( MEL_IOS ) || defined( MEL_MACOSX )
    kern_return_t err = mach_timebase_info(
        &mTimeBase ); // this converts mach_time to nanoseconds
    if ( err )
    {
        mTimeBase.numer = 0;
        mTimeBase.denom = 1;
    }
    // nanoseconds to seconds :)
    if ( mTimeBase.numer >= 1000000 )
        mTimeBase.numer /= 1000000;
    else
        mTimeBase.denom *= 1000000;
    mReference = mach_absolute_time();
#elif defined( MEL_ANDROID )
    timespec ts;
    mReference = 0;
    if ( !clock_gettime( CLOCK_MONOTONIC, &ts ) )
        mReference = ts.tv_sec * 1000 + ( ts.tv_nsec / 1000000 );
#endif
    mStartTime = 0;
}

void Timer::pause()
{
    if ( mState == ACTIVE )
    {
        mMsActive = getMilliseconds();
        mState = PAUSED;
    }
}
void Timer::resume()
{
    if ( mState == PAUSED )
    {
        // chage reference time to current moment
#ifdef WIN32
        QueryPerformanceCounter( (LARGE_INTEGER*)&mReference );
#elif defined( MEL_IOS ) || defined( MEL_MACOSX )
        mReference = mach_absolute_time();
#elif defined( MEL_LINUX ) || defined( MEL_ANDROID )
        timespec ts;
        if ( !clock_gettime( CLOCK_MONOTONIC, &ts ) )
            mReference = ts.tv_sec * 1000 + ( ts.tv_nsec / 1000000 );
#endif
        mState = ACTIVE;
    }
}
