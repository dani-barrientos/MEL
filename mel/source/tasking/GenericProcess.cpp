#include <tasking/GenericProcess.h>
using ::mel::tasking::EGenericProcessResult;
using mel::tasking::GenericProcess;

GenericProcess::GenericProcess()
    : mProcessCallback( 0 ), mUpdateResult( EGenericProcessResult::CONTINUE )
{
}
GenericProcess::~GenericProcess() {}

void GenericProcess::onInit( uint64_t msegs ) {}
bool GenericProcess::onKill()
{
    return mUpdateResult == ( EGenericProcessResult::KILL ) || !mKillCallback ||
           mKillCallback();
}
void GenericProcess::onUpdate( uint64_t msegs )
{
    if ( mUpdateResult == EGenericProcessResult::CONTINUE )
    {
        mUpdateResult = mProcessCallback( msegs, this );
        if ( mUpdateResult == EGenericProcessResult::KILL )
        {
            kill();
        }
    }
}
