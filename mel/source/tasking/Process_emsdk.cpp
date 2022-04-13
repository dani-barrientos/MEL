
#include <tasking/Process.h>
using mel::tasking::Process;
#include <text/logger.h>  //for debug

volatile void Process::checkMicrothread( uint64_t msegs )
{	
	// if ( !mFiberInited )
	// {
	// 	mFiberInited = true;
	// 	emscripten_fiber_init( &mFiberData,nullptr,nullptr,mStack,)
	// }
	
	_execute(msegs);
}

static bool sInit = false; //for debugging purposes
		EM_JS(void, start_timer, (), {
			Module.timer = false;
			setTimeout(function() {
				Module.timer = true;
				console.log('HOLA');
			}, 500);
		});
void Process::_switchProcess( ) OPTIMIZE_FLAGS
{
	//mel::text::debug("Process::_switchProcess( )");
	//lo que no me cuadra del uso de fibras es que al hacer el switch hay que decirle la fibra destino...así que imposible cuadrarlo
	//@todo

	necesito tener bien claro como se haría en javascript completo un sistema de tareas y luego ver si cuadra
	if ( sInit == false )un sistema de tareas y luego ver si cuadra
	{
		sInit = true;
		mel::text::debug("Process::_switchProcess Init");		
	//	start_timer();

	}
}