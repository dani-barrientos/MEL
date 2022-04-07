#include <tasking/Process.h>
#include <tasking/ProcessScheduler.h>
using mel::tasking::Process;
using mel::tasking::MThreadAttributtes;
#pragma optimize("",off)
extern "C" void _checkMicrothread(MThreadAttributtes*,uint64_t msegs,Process* p,void* _executePtr );
extern "C" void _switchMT(MThreadAttributtes*);

volatile void Process::checkMicrothread( uint64_t msegs )
{	
	auto ptr = &Process::_execute;	
	_checkMicrothread(this,msegs,this,(void*&)ptr);	
}
void Process::_switchProcess( ) OPTIMIZE_FLAGS
{
	Process* p = ProcessScheduler::getCurrentProcess().get();
	_switchMT(p);
}
#pragma optimize("",on)

