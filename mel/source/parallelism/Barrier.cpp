#include <parallelism/Barrier.h>
using mel::parallelism::Barrier;
using mel::parallelism::BarrierData;

void BarrierData::set()
{
    volatile auto protectMe = shared_from_this();
    std::scoped_lock<std::mutex> lck(mCS);
    if ( --mActiveWorkers == 0 ) 
    {				
        triggerCallbacks(*this);
    }
}
