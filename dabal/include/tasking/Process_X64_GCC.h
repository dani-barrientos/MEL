#pragma once

namespace tasking
{
    struct MThreadAttributtes;
}
using tasking::MThreadAttributtes;
#ifdef _MSC_VER
extern "C"   void resizeStack(  MThreadAttributtes* process,  unsigned int newSize ) ;
#else
extern "C"   void  resizeStack(  MThreadAttributtes* process,  unsigned int newSize );
#endif
///@cond HIDDEN_SYMBOLS
namespace tasking
{
    struct MThreadAttributtes 
    {
    public:
        volatile void* mIniSP;
        volatile void* mStackEnd;
        volatile unsigned char* mStack;
        volatile void* mIniRBP;
        volatile void* mIniRBX;
        volatile void* mRegisters[4]; //r12.r15
        volatile bool mSwitched;
        volatile uint16_t mFpcsr;  //fpu status register
        volatile unsigned int mMxcsr; //mmx control register
        volatile unsigned int mStackSize;
        volatile unsigned int mCapacity;
    };
}
///@endcond
