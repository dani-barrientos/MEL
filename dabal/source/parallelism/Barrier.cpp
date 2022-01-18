#include <parallelism/Barrier.h>
using parallelism::Barrier;
using parallelism::BarrierData;

void BarrierData::set()
{
    volatile auto protectMe = shared_from_this();
    ::core::Lock lck(mCS);
    if ( --mActiveWorkers == 0 ) 
    {				
        triggerCallbacks(*this);
    }
}
