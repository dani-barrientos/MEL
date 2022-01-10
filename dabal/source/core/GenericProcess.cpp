#include <core/GenericProcess.h>
using core::GenericProcess;
using ::core::EGenericProcessState;
using ::core::EGenericProcessResult;

DABAL_CORE_OBJECT_TYPEINFO_IMPL(GenericProcess,Process);
GenericProcess::GenericProcess() :
	mProcessCallback( 0 )
	,mCurrentState( EGenericProcessState::INIT )
	,mUpdateResult( EGenericProcessResult::CONTINUE )
	,mAutoKill( false )
	{
	}
GenericProcess::~GenericProcess()
{
}

void GenericProcess::onInit(uint64_t msegs)
{

/*	mCurrentState = INIT;
	mKillAccepted = false; //it's important because maybe mProcessCallback use microthread behaviour
	mKillAccepted = (*mProcessCallback)( msegs, this,INIT );
	if ( mKillAccepted )
	{
		kill();		
		mCurrentState = KILL;//because onInit doesn't generate onKill call TODO �cambiarlo?
	}else
	{
		mCurrentState = RUN;
	}
	*/
}
bool GenericProcess::onKill()
{

	mCurrentState = EGenericProcessState::KILL;
	return mUpdateResult==(EGenericProcessResult::KILL) || mAutoKill;
}
void GenericProcess::onUpdate(uint64_t msegs)
{
	if ( mUpdateResult == EGenericProcessResult::CONTINUE)
	{
		mUpdateResult = mProcessCallback( msegs, this,mCurrentState );
		if ( mCurrentState == EGenericProcessState::INIT )
			mCurrentState = EGenericProcessState::RUN;
		if (  mUpdateResult == EGenericProcessResult::KILL  )
		{
			kill();
		}
	}
	
}
/*
void GenericProcess::setProcessCallback(std::function< bool(uint64_t, Process*, EGenericProcessState)>&& f)
{
	delete mProcessCallback;
	mProcessCallback = new GenericCallback(::std::move(f), ::core::use_function);
}
void GenericProcess::setProcessCallback(const std::function< bool(uint64_t, Process*, EGenericProcessState)>& f)
{
	delete mProcessCallback;
	mProcessCallback = new GenericCallback(f, ::core::use_function);
}
void GenericProcess::setProcessCallback(std::function< bool(uint64_t, Process*, EGenericProcessState)>& f)
{
	delete mProcessCallback;
	mProcessCallback = new GenericCallback(f, ::core::use_function);
}
*/