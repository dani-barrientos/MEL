#pragma once
/*
 * SPDX-FileCopyrightText: 2010,2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
namespace mel
{
	namespace tasking
	{
		struct MThreadAttributtes;
	}
	using mel::core::MThreadAttributtes;

	///@cond HIDDEN_SYMBOLS
	extern "C"  void __attribute__ ((noinline)) mel_tasking_resizeStack(  MThreadAttributtes* process,  unsigned int newSize );

	namespace tasking
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
	///@endcond
}