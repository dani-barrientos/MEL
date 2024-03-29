#pragma once
/*
 * SPDX-FileCopyrightText: 2013,2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
///@cond HIDDEN_SYMBOLS
namespace mel
{	
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
			volatile void*	mStackEnd;
			volatile unsigned char*	mStack;
			volatile unsigned int	mStackSize;
			volatile unsigned int	mCapacity;
			volatile double	mCoRegisters[16];
	#elif defined(__aarch64__)
			volatile bool mSwitched;
			volatile unsigned int mCapacity;
			volatile unsigned int mStackSize;
			volatile void*	mLR;
			volatile void* mIniSP;
			volatile void* mStackEnd;
			volatile int64_t mRegisters[11]; //x19-x29. Lr is not necessary because it's saved in stack in function prolog
			volatile int64_t mVRegisters[8];  //64 bottom bits of v8-v15
			volatile unsigned char* mStack;

	#endif
		};
	}
}
extern "C"  void mel_tasking_resizeStack(  mel::tasking::MThreadAttributtes* process,  unsigned int newSize ) ;
///@endcond

