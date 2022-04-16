
#include <tasking/Process.h>
using mel::tasking::Process;
#include <text/logger.h>  //for debug
#include <tasking/ProcessScheduler.h>
EMSCRIPTEN_KEEPALIVE
extern "C" bool _sExecute( int msegs) //parece que tengo que devolver algo
{
	mel::text::info("_sExecute {}",msegs);
	auto p = mel::tasking::ProcessScheduler::getCurrentProcess();
	p->_execute(msegs);
	return true;
}
//version cutre pasando puntero @todo luego usar uint64_t

EM_ASYNC_JS(void, doExecute, (int msegs), {
 
 	out("doExecute "+msegs);
	await Module.ccall("_sExecute",'boolean',['number'],[msegs]); //@todo use cwrap
});
volatile void Process::checkMicrothread( uint64_t msegs )
{	
	// if ( !mFiberInited )
	// {
	// 	mFiberInited = true;
	// 	emscripten_fiber_init( &mFiberData,nullptr,nullptr,mStack,)
	// }
	text::info("Process::checkMicrothread");
	doExecute((int)msegs); 
	//_execute(msegs);
	text::info("Process::checkMicrothread  after");
}

EM_ASYNC_JS(int, doSwitch, (), {
  out("waiting for a fetch");
  let p = new Promise( 
            function (resolve)
            {
                //self.resolveCb = resolve;               
            });
  await p;
  //@todo el probleam es como uso guardo el resolveCb
  out("got the fetch response");
  // (normally you would do something with the fetch here)
  return 1;
});
EM_ASYNC_JS(void, test, (), {
  console.log("JS test\n");
  let p = Promise.resolve(1);             
  await p;	  
});
		// EM_JS(void, start_timer, (), {
		// 	Module.timer = false;
		// 	setTimeout(function() {
		// 		Module.timer = true;
		// 		console.log('HOLA');
		// 	}, 500);
		// });
void Process::_switchProcess( ) OPTIMIZE_FLAGS
{
	//mel::text::debug("Process::_switchProcess( )");
	//lo que no me cuadra del uso de fibras es que al hacer el switch hay que decirle la fibra destino...así que imposible cuadrarlo
	//@todo
	mel::text::debug("Process::_switchProcess");		
//	doSwitch();	
	//test();
	mel::text::debug("After Process::_switchProcess");
}