
#include <tasking/Process.h>
using mel::tasking::Process;
volatile void Process::checkMicrothread( uint64_t msegs )
{	
	_execute(msegs);
}
void Process::_switchProcess( ) OPTIMIZE_FLAGS
{
	//@todo
}