#include <core/GenericProcess.h>
using core::GenericProcess;

DABAL_CORE_OBJECT_TYPEINFO_IMPL(GenericProcess,Process);
GenericProcess::GenericProcess() :
	mProcessCallback( 0 )
	,mCurrentState( INIT )
	,mKillAccepted( false )
	,mAutoKill( false )
	{
	}
GenericProcess::~GenericProcess()
{
	//OutputDebugStringA( "DESTRUYO GENERICPROCESS" ); problemas de corrupcion de memoria en AQTime
	//OutputDebugStringA( "VOY A DESTRUIR CALLBACK" );
	delete mProcessCallback;
	//OutputDebugStringA( "DESTRUIDO CALLBACK" );
	
}

void GenericProcess::onInit(uint64_t msegs)
{

/*	mCurrentState = INIT;
	mKillAccepted = false; //it's important because maybe mProcessCallback use microthread behaviour
	mKillAccepted = (*mProcessCallback)( msegs, this,INIT );
	if ( mKillAccepted )
	{
		kill();		
		mCurrentState = KILL;//because onInit doesn't generate onKill call TODO ¿cambiarlo?
	}else
	{
		mCurrentState = RUN;
	}
	*/
}
bool GenericProcess::onKill()
{

	mCurrentState = KILL;
	return mKillAccepted || mAutoKill/* || (mKillAccepted = (*mProcessCallback)( getUpdateTime(), this,mCurrentState ) )*/;
}
void GenericProcess::update(uint64_t msegs)
{
	if ( !mKillAccepted )
	{
		mKillAccepted = (*mProcessCallback)( msegs, this,mCurrentState );
		if ( mCurrentState == INIT )
			mCurrentState = RUN;
		if (  mKillAccepted  )
		{
			kill();
		}
	}
	
}
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