#pragma once
#include <MelLibType.h>
#include <stdint.h>
#ifdef WIN32
#elif defined (MEL_IOS) || defined(MEL_MACOSX)
#include <mach/mach_time.h>
#elif defined (MEL_LINUX) || defined (MEL_ANDROID)
#include <time.h>
#endif

#include <cassert>
struct tm; //predeclaration
namespace mel
{
	namespace core
	{
		class MEL_API Timer 
		{

		public:
			/**
			* Creates a new timer.
			* The initial count value is reset to 0
			*/
			Timer();
			virtual ~Timer();

			/**
			* Resets the timer.
			* Sets the internal counter back to 0.
			*/
			virtual void reset();
			/**
			* pauses timer. All next time request will return pause time
			*/
			void pause();
			void resume();
			/**
			* return current time as accuracy milliseconds
			* @return milliseconds since starttime, which by default is 0
			*/
			inline uint64_t getMilliseconds( ) const;
			/**
			* return last measured time. 
			*/
			inline uint64_t getLastMilliseconds() const;
			inline bool getPaused() const;
			/**
			* set start time (msecs) from when timer counts
			*/
			inline uint64_t getStartTime() const;
			inline void setStartTime( uint64_t );
			

		protected:
			uint64_t mReference; //reference time when Timer is created, units depends on platform
			uint64_t mStartTime;
	#ifdef WIN32
			uint64_t mFrequency;
			mutable uint64_t mLastTime; //!last measured time in milliseconds
	#elif defined (MEL_IOS) || defined(MEL_MACOSX) 
			mach_timebase_info_data_t mTimeBase;
			mutable uint64_t mLastTime;
	#elif defined(MEL_LINUX) || defined(MEL_ANDROID)
			mutable uint64_t mLastTime;
	#endif
			uint64_t   mMsActive; //! active time in milliseconds
		private:
			enum { ACTIVE, PAUSED } mState;
		};
		uint64_t Timer::getMilliseconds( ) const
		{
			uint64_t result;
			if ( mState == ACTIVE )
			{
			#ifdef WIN32
				uint64_t tmp;
				QueryPerformanceCounter((LARGE_INTEGER*)&tmp);
				result = (tmp - mReference)*1000/mFrequency;  //because counter/frecuency = seconds
			#elif defined(MEL_IOS) || defined(MEL_MACOSX)
				uint64_t tmp=mach_absolute_time();
				uint64_t elapsed=tmp-mReference;
				result = (elapsed * mTimeBase.numer )/mTimeBase.denom;
			#elif defined(MEL_LINUX) || defined(MEL_ANDROID)
				timespec ts;
				uint64_t tmp;
				int chk(clock_gettime(CLOCK_MONOTONIC, &ts));
				#pragma unused(chk)
				assert(!chk);
				tmp=ts.tv_sec * 1000 + ts.tv_nsec/1000000;
				result=tmp-mReference;
			#endif		
			}else
			{
				//it's paused
				result = 0;
			}
			mLastTime = result  + mMsActive + mStartTime;
			return mLastTime;
		}
		uint64_t Timer::getLastMilliseconds() const
		{
			//@todo no me gusta nada esta conversion y tal vez alg�n d�a tengamos problemas, pero devolver 64bits es
			//muy ineficiente ahrora
			return (unsigned long)mLastTime;
		}
		bool Timer::getPaused() const
		{
			return mState == PAUSED;
		}
		uint64_t Timer::getStartTime() const
		{
			return mStartTime;
		}
		void Timer::setStartTime( uint64_t v )
		{
			mStartTime = v;
		}
	}
}