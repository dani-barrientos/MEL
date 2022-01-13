#pragma once

namespace tasking
{
    struct MThreadAttributtes;
}
using tasking::MThreadAttributtes;
#ifdef _MSC_VER
	extern "C"   void resizeStack(  MThreadAttributtes* process,  unsigned int newSize ) ;
#else
	extern "C"   void  resizeStack(  MThreadAttributtes* process,  unsigned int newSize );
#endif
namespace tasking
{
    struct MThreadAttributtes
    {
     public:
        volatile bool mSwitched;
        volatile void* mActualSP;
        volatile void* mIniSP;
        volatile void* mStackEnd;
        volatile unsigned int mMXCSR;
        volatile unsigned short	  mStackSize;
        volatile unsigned char* mStack;
        volatile void* mIniBP; 
        volatile unsigned int mCapacity;
    };
}

