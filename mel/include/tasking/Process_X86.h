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
        public:
            volatile bool mSwitched;
            volatile void* mActualSP;
            volatile void* mIniSP;
            volatile void* mStackEnd;
            volatile unsigned short	 mStackSize;
            volatile unsigned char* mStack;
            volatile void* mIniBP; 
            volatile unsigned int mCapacity;
        };
    }    
}
extern "C"  void mel_tasking_resizeStack( ::mel::tasking::MThreadAttributtes* process,  unsigned int newSize ) ;
///@endcond
