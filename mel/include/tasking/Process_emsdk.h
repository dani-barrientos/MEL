#pragma once
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
        //for compatibility, but don't do any job
        struct MThreadAttributtes 
        {
            volatile unsigned char* mStack;        
            volatile void* mStackEnd;
            volatile bool mSwitched;
            volatile unsigned int mStackSize;
            volatile unsigned int mCapacity;                
        };
    }
    ///@endcond
}