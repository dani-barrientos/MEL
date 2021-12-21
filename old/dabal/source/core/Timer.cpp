#include <core/Timer.h>
using core::Timer;
#include <iostream>
using std::cout;
#include <ctime>


static ::core::CriticalSection& getCS() {
	//critical section to access C time funcitons (gmtime and localtime)
	static ::core::CriticalSection MYCS;
	return MYCS;
}

Timer::Timer() :
	mStartTime(0),
	mState( ACTIVE )
	,mMsActive( 0 )

{
	reset();
}

Timer::~Timer() {
	//OK. Do nothing
}

void Timer::reset() {
#ifdef _WINDOWS
	QueryPerformanceFrequency((LARGE_INTEGER*)&mFrequency);
	QueryPerformanceCounter((LARGE_INTEGER*)&mReference);
#elif defined(_IOS) || defined(_MACOSX)
	kern_return_t err = mach_timebase_info( &mTimeBase );	//this converts mach_time to nanoseconds
	if (err) {
		mTimeBase.numer=0;
		mTimeBase.denom=1;
	}
	//nanoseconds to seconds :)
	if (mTimeBase.numer>=1000000)
		mTimeBase.numer/=1000000;
	else
		mTimeBase.denom*=1000000;
	mReference=mach_absolute_time();
#elif defined(_ANDROID)
	timespec ts;
	mReference=0;
	if (!clock_gettime(CLOCK_MONOTONIC,&ts))
		mReference=ts.tv_sec * 1000 + (ts.tv_nsec/1000000);
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
		//chage reference time to current moment
#ifdef WIN32
		QueryPerformanceCounter((LARGE_INTEGER*)&mReference);
#elif defined(_IOS) || defined(_MACOSX)
		mReference=mach_absolute_time();
#elif defined(_ANDROID)
	timespec ts;
	if (!clock_gettime(CLOCK_MONOTONIC,&ts))
		mReference=ts.tv_sec * 1000 + (ts.tv_nsec/1000000);
#endif
		mState = ACTIVE;
	}

}

#ifdef _WINDOWS
void UnixTimeToFileTime(time_t t, FILETIME& ft) {
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;
	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	ft.dwLowDateTime = (DWORD)ll;
	ft.dwHighDateTime = ll >> 32;
}
void UnixTimeToSystemTime(time_t t, SYSTEMTIME& pst) {
	FILETIME ft;

	UnixTimeToFileTime(t, ft);
	FileTimeToSystemTime(&ft, &pst);
}
static int32_t MONTH_DAYS[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
static bool isLeapYear(int year) {
	return (((year % 4) == 0) && (((year % 100) != 0) || ((year % 400) == 0)));
}
time_t FileTimeToUnixTime(const FILETIME& ft) {
	LARGE_INTEGER ull;
	ull.LowPart = ft.dwLowDateTime;
	ull.HighPart = ft.dwHighDateTime;
	TIME_ZONE_INFORMATION tzi;
	GetTimeZoneInformation(&tzi);
	return (ull.QuadPart / 10000000ULL) - 11644473600ULL + (tzi.Bias*60);
}
time_t SystemTimeToUnixTime(const SYSTEMTIME& st) {
	FILETIME ft;
	if (SystemTimeToFileTime(&st, &ft)) {
		return FileTimeToUnixTime(ft);
	}
	else {
		return 0;
	}
}
//month: 0..11
//day: 1..31
static int32_t _dayOfYear(int year, int month, int day) {
	int32_t yd=0;
	for (int32_t m=0;m<month;++m) {
		yd += MONTH_DAYS[m];
	}
	yd += day;
	if ((month > 1) && isLeapYear(year)) {
		yd++;
	}
	return yd;
}
//month: 0..11
static int _daysOfMonth(int32_t year, int32_t month) {
	int32_t d = MONTH_DAYS[month];
	if ((month == 1) && isLeapYear(year)) {
		++d;
	}
	return d;
}

void SystemTimeToTM(const SYSTEMTIME& st, struct tm& t) {
	t.tm_hour = st.wHour;
	t.tm_min = st.wMinute;
	t.tm_mday = st.wDay;
	t.tm_mon = st.wMonth-1;
	t.tm_sec = st.wSecond;
	t.tm_year = st.wYear - 1900;
	t.tm_wday = st.wDayOfWeek;
	t.tm_yday = _dayOfYear(st.wYear, t.tm_mon, st.wDay);
	t.tm_isdst = -1;
}
//month: 0..11
//day: 1..31
static int checkMaxDay(int32_t year, int32_t month, int32_t day) {
	int32_t dMax = _daysOfMonth(year, month);
	return (day <= dMax)? day : dMax;
}
static void TMToSystemTime(const struct tm& t,SYSTEMTIME& st) {
	st.wYear = t.tm_year + 1900;
	st.wMonth = t.tm_mon + 1;
	st.wDay = checkMaxDay(st.wYear, t.tm_mon, t.tm_mday);
	st.wDayOfWeek = t.tm_wday;
	st.wHour = t.tm_hour;
	st.wMinute = t.tm_min;
	st.wSecond = t.tm_sec;
	st.wMilliseconds = 0;
}
#endif

static struct tm* _gmtime(const time_t* t, struct tm& output) {
	struct tm* aux = ::gmtime(t);
	if (aux) {
		output = *aux;
		return &output;
	}
	else {
		return nullptr;
	}
}
struct tm* Timer::gmtime(const time_t* t, struct tm& output)
{
	if (!t) {
		return nullptr;
	}
	
	::core::Lock lck(getCS());//(sTimeCS);
#ifndef _WINDOWS
	return _gmtime(t, output);
#else
	if ((*t) >= 0) {
		return _gmtime(t, output);
	}
	else {
		SYSTEMTIME st;
		UnixTimeToSystemTime(*t, st);
		SystemTimeToTM(st, output);
		return &output;
	}
#endif
}

static struct tm* _localtime(const time_t* t, struct tm& output) {
	struct tm* aux = ::localtime(t);
	if (aux) {
		output = *aux;
		return &output;
	}
	else {
		return nullptr;
	}
}

struct tm* Timer::localtime(const time_t* t, struct tm& output) {
	if (!t) return nullptr;
	::core::Lock lck(getCS());// sTimeCS);
#ifndef _WINDOWS
	return _localtime(t, output);
#else
	if ( (*t) >= 0) {
		return _localtime(t, output);
	}
	else {
		TIME_ZONE_INFORMATION tzi;
		DWORD dst = GetTimeZoneInformation(&tzi);
		time_t tmp = (*t) - tzi.Bias * 60;
		Timer::gmtime(&tmp, output);
		return &output;
	}
#endif
}

static time_t _mktime(struct tm* t) {
	return ::mktime(t);
}

time_t Timer::mktime(struct tm* date) {
	if (!date) return 0;
	::core::Lock lck(getCS());// sTimeCS);

#ifndef _WINDOWS
	return _mktime(date);
#else
	int32_t y = date->tm_year + 1900;
	if (y >= 1970) {
		return _mktime(date);
	}
	else {
		SYSTEMTIME st;
		TMToSystemTime(*date, st);
		if (st.wYear != (date->tm_year + 1900)) {
			date->tm_year = st.wYear - 1900;
		}
		if (st.wMonth != (date->tm_mon + 1)) {
			date->tm_mon = st.wMonth - 1;
		}
		if (st.wDay != date->tm_mday) {
			date->tm_mday = st.wDay;
		}
		return SystemTimeToUnixTime(st);
	}
#endif
}