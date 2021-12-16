#pragma once
TAL VEZ NO HAGA FALTA
namespace core
{
    struct MThreadAttributtes;
}
using core::MThreadAttributtes;
#ifdef _MSC_VER
	extern "C"   void resizeStack(  MThreadAttributtes* process,  unsigned int newSize ) ;
#else
	extern "C"   void  resizeStack(  MThreadAttributtes* process,  unsigned int newSize );
#endif
namespace core
{
    struct MThreadAttributtes
    {
     public:
        volatile bool mSwitched;
        //volatile void* mLR;
        volatile void* mActualSP;// +4
        volatile void* mIniSP;//+8
        volatile void* mStackEnd; //+12
        volatile unsigned short	  mStackSize;//+16
        volatile unsigned char* mStack; //+20
        volatile void* mIniBP; //+24
        volatile unsigned int mCapacity;
    };
}

