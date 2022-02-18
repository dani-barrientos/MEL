#pragma once

namespace tasking
{
	struct MThreadAttributtes;
}
using tasking::MThreadAttributtes;

extern "C"  void __attribute__ ((noinline)) resizeStack(  MThreadAttributtes* process,  unsigned int newSize );

///@cond HIDDEN_SYMBOLS
namespace tasking
{
	struct MThreadAttributtes
	{
#ifdef __arm__
		volatile bool	mSwitched;
		volatile void*	mLR; 
		volatile void*	mActualSP;
		volatile void*	mIniSP;
		volatile int	mRegisters[13];
		volatile double	mCoRegisters[16];
		volatile void*	mStackEnd;
		volatile unsigned char*	mStack;
		volatile unsigned int	mStackSize;
		volatile unsigned int	mCapacity;
#elif defined(__aarch64__)
		volatile bool mSwitched;
		volatile void*	mLR;
		volatile void* mIniSP;
		volatile int64_t mRegisters[11]; //x19-x29. Lr is not necces	ary because it's saved in stack in function prolog
	    volatile int64_t mVRegisters[8];  //64 bottom bits of v8-v15
		volatile void* mStackEnd;
		volatile unsigned char* mStack;
		volatile unsigned int mCapacity;
		volatile unsigned int mStackSize;
#endif
	};
}
///@endcond