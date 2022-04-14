#pragma once
/*
 * SPDX-FileCopyrightText: 2005,2022 Daniel Barrientos <danivillamanin@gmail.com>
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
		//protected:
		public:
			//bool	mSwitched; //+0
			volatile unsigned int mSwitched; //+0 TODo cuando controle bien poner un bool
			volatile void*	mLR; //+4
			/**
			* stack pointer at process begin
			*/
			volatile void* mActualSP; //+8
			volatile void* mIniSP; //+12
			//void* mIniBP;
			volatile void* mStackEnd; //+16
			volatile unsigned int	  mStackSize; //+20

			/**
			* stack for this process
			*/
			volatile unsigned char* mStack; //+24
			volatile unsigned char mCapacity;
			volatile bool mSleeped;
			bool    mReturnInmediately;
			unsigned int mRealPeriod;
		};
	}
}
extern "C" volatile void __attribute__ ((noinline)) mel_tasking_resizeStack(  ::mel::tasking::MThreadAttributtes* process,  unsigned int newSize );
///@endcond
