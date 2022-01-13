#pragma once


namespace tasking
{
	struct MThreadAttributtes;
}
using tasking::MThreadAttributtes;

extern "C"  void __attribute__ ((noinline)) resizeStack(  MThreadAttributtes* process,  unsigned int newSize );
namespace tasking
{
	struct MThreadAttributtes
	{	
		volatile bool mSwitched;       
		volatile void*	mLR; 
		volatile void* mIniSP;
		volatile int64_t mRegisters[11]; //x19-x29. Lr is not neccesary because it's saved in atack in function prolog
        volatile int64_t mVRegisters[8];  //64 bottom bits of v8-v15
		volatile void* mStackEnd; 
		volatile unsigned int	  mStackSize;
		volatile unsigned char* mStack; 
		volatile unsigned int mCapacity;
 };
}
