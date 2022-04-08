#ifdef _WINDOWS
/*
 * SPDX-FileCopyrightText: 2010,2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
//#include <Windows.h>
namespace mel
{
	namespace core {

	CriticalSection::CriticalSectionData::CriticalSectionData() {
		InitializeCriticalSection(&mCS);
	}

	CriticalSection::CriticalSectionData::~CriticalSectionData() {
		DeleteCriticalSection(&mCS);
	}

	void CriticalSection::CriticalSectionData::enter() 
	{
		EnterCriticalSection(&mCS);
	}

	bool CriticalSection::CriticalSectionData::tryEnter() {
		BOOL ret=TryEnterCriticalSection(&mCS);
		return ret==TRUE;
	}

	void CriticalSection::CriticalSectionData::leave() {
		LeaveCriticalSection(&mCS);
	}

	//End namespace	
	}
}
#endif