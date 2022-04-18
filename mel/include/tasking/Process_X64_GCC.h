#pragma once
/*
 * SPDX-FileCopyrightText: 2005,2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
namespace mel
{
    namespace tasking
    {
        struct MThreadAttributtes;
    }
    using mel::tasking::MThreadAttributtes;
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
            // volatile uint16_t mFpcsr;  //fpu status register
            // volatile unsigned int mMxcsr; //mmx control register
            volatile unsigned int mStackSize;
            volatile unsigned int mCapacity;
        };
    }
    ///@endcond
}
///@cond HIDDEN_SYMBOLS
extern "C"  void mel_tasking_resizeStack(  mel::tasking::MThreadAttributtes* process,  unsigned int newSize );
///@endcond