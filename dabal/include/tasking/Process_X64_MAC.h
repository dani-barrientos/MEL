#pragma once
//same code as in GCC/Clang. just in case a different file is hold to adress possible diferences in compiler
#include <tasking/Process_X64_GCC.h>
/*
namespace tasking
{
    struct MThreadAttributtes;
}
using tasking::MThreadAttributtes;
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
        volatile unsigned int mStackSize;
        volatile unsigned int mCapacity;
    };
}
///@endcond
*/


