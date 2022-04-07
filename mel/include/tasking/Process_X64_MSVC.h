#pragma once
namespace mel
{
    namespace tasking
    {
        struct MThreadAttributtes;
    }
    using mel::tasking::MThreadAttributtes;
    extern "C"   void resizeStack(  MThreadAttributtes* process,  unsigned int newSize ) ;
    #include <xmmintrin.h>

    ///@cond HIDDEN_SYMBOLS
    namespace tasking
    {
        struct MThreadAttributtes 
        {
        public:

            volatile void* mIniSP;
            volatile void* mStackEnd;
            volatile unsigned char* mStack;        
            volatile void* RBP;
            volatile void* RBX;
            volatile void* RSI;
            volatile void* RDI;
            volatile void* mRegisters[4]; //r12.r15
            volatile __m128 mXMM[10] alignas(16); //XMM6..XMM15
            volatile bool mSwitched;
            volatile uint16_t mFpcsr;  //fpu status register
            volatile unsigned int mMxcsr; //mmx control register
            volatile unsigned int mStackSize;
            volatile unsigned int mCapacity;                
        };
    }
    ///@endcond
}

