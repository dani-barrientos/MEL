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
		volatile void* mActualSP; 
		volatile void* mIniSP; 
		volatile int mRegisters[13];
        volatile double mCoRegisters[16];
		volatile void* mStackEnd; 
		volatile unsigned int	  mStackSize;
		volatile unsigned char* mStack; 
		volatile unsigned int mCapacity;
	};
}
