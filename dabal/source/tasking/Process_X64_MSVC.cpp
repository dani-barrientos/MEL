#ifdef _MSC_VER

#include <tasking/Process.h>
using tasking::Process;
using tasking::MThreadAttributtes;

#include <iostream>
#include <tasking/ProcessScheduler.h>
#pragma optimize("",off)
extern "C" void _checkMT(MThreadAttributtes*);
extern "C" void _checkMicrothread(MThreadAttributtes*,uint64_t msegs,Process* p,void* _executePtr );
extern "C" void _switchMT(MThreadAttributtes*);
/*
extern "C" void Process_execute(MThreadAttributtes* obj, uint64_t msegs)
{
	// con este mecanismo ya tengo todo el codigo dentro, el problema 
	// es que tengo que hacer public el _execute.
	// Si resuelvo el namemangling, creo que puedo llamar directamente al _execute desde el asm
	// POSIBILIDADES:
	//  - DEJAR EL _EXECUTE PUBLIC->NO ME GUSTA
	//  -
	Process* ptr = static_cast<Process*>(obj);
	ptr->_execute(msegs);
}
*/
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
#endif

