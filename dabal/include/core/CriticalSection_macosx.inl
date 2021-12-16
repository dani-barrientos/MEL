#if defined (_MACOSX) || defined(_IOS) || defined(_ANDROID)
#include <errno.h>
#include <assert.h>

namespace core {

CriticalSection::CriticalSectionData::CriticalSectionData() {
	pthread_mutexattr_t CSATT;

	int ret=pthread_mutexattr_init(&CSATT);
	assert(!ret && "Unable to initialize mutex attribtues!");

	//Note: recursive mutex -> it's ok for the same thread to call CriticalSection::enter
	//several times, provided it calls CriticalSection::leave the same number ot times
	ret=pthread_mutexattr_settype(&CSATT,PTHREAD_MUTEX_RECURSIVE);
	assert(!ret && "Unable to set mutex type!");
	
	ret=pthread_mutex_init(&mCS,&CSATT);
	assert(!ret && "Unable to initialize mutex!");
	
	ret=pthread_mutexattr_destroy(&CSATT);
	assert(!ret && "Error destroying mutex attribute!");
}

CriticalSection::CriticalSectionData::~CriticalSectionData() {
	int ret=pthread_mutex_destroy(&mCS);
	assert(!ret && "Error destroying mutex!");
//!@note: to avoid the "unused variable" warning
#ifndef WIN32
#pragma unused(ret)
#else
	ret;
#endif
}

void CriticalSection::CriticalSectionData::enter() {
	int ret=pthread_mutex_lock(&mCS);
	assert(!ret && "Error locking mutex!");
//!@note: to avoid the "unused variable" warning
#ifndef WIN32
#pragma unused(ret)
#else
	ret;
#endif
}

bool CriticalSection::CriticalSectionData::tryEnter() {
	int ret=pthread_mutex_trylock(&mCS);
	return ret!=EBUSY;
}

void CriticalSection::CriticalSectionData::leave() {
	int ret=pthread_mutex_unlock(&mCS);
	assert(!ret && "Error unlocking mutex!");
    //!@note: to avoid the "unused variable" warning
#ifndef WIN32
#pragma unused(ret)
#else
	ret;
#endif

}

//End namespace	
}
#endif