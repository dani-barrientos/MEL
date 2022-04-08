/*
 * SPDX-FileCopyrightText: 2010,2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
#if defined (MEL_LINUX) || defined (MEL_MACOSX) || defined(MEL_ANDROID) || defined (MEL_IOS)
#include <errno.h>
#include <assert.h>
#include <pthread.h>
namespace mel
{
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
		if ( ret!=0 )
			ret = 6;
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
		//@note: if current thread already owns the mutext shouldn´t be considered error
		assert( (ret!=EINVAL && ret != EAGAIN) && "Error locking mutex!");
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
		//@note calling leave to a non-owner mutex shouldn't be considered error
		assert(ret!=EINVAL && "Error unlocking mutex!");
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
}