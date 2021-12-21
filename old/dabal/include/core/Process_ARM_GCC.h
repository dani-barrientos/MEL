#pragma once


namespace core
{
	struct MThreadAttributtes;
}
using core::MThreadAttributtes;

//POR ÑAPA DEL AIRPLAY
#ifndef I3D_ARCH_X86
	extern "C" volatile void __attribute__ ((noinline)) resizeStack(  MThreadAttributtes* process,  unsigned int newSize );
#else
extern "C" volatile void resizeStack(  MThreadAttributtes* process,  unsigned int newSize );
#endif
namespace core
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
