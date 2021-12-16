#pragma once


namespace core
{
	struct MThreadAttributtes;
}
using core::MThreadAttributtes;

extern "C"  void __attribute__ ((noinline)) resizeStack(  MThreadAttributtes* process,  unsigned int newSize );
namespace core
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
