#pragma once

namespace mel
{
	namespace tasking
	{
		struct MThreadAttributtes;
	}
	using mel::tasking::MThreadAttributtes;

	extern "C" volatile void __attribute__ ((noinline)) resizeStack(  MThreadAttributtes* process,  unsigned int newSize );

	///@cond HIDDEN_SYMBOLS
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
	///@endcond
}