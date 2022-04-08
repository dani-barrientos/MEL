#pragma once
/*
 * SPDX-FileCopyrightText: 2005,2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
#include <xmmintrin.h>
///@cond HIDDEN_SYMBOLS
namespace mel
{
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
}
extern "C"  void mel_tasking_resizeStack(  mel::tasking::MThreadAttributtes* process,  unsigned int newSize ) ;
///@endcond

