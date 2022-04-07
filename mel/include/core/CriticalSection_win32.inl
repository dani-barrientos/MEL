#ifdef _WINDOWS
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