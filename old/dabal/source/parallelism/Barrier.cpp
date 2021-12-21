#include <parallelism/Barrier.h>
using parallelism::Barrier;
using parallelism::BarrierData;


Barrier::Barrier( BarrierData* data ):mData(data){}
