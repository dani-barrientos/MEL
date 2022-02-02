#include "test.h"
#include <core/GenericThread.h>
using core::GenericThread;
using namespace std;
#include <TestManager.h>
using tests::TestManager;
#include <spdlog/spdlog.h>
#include <CommandLine.h>
#include <mpl/LinkArgs.h>
#include <mpl/Ref.h>
#include <string>
#include <tasking/Process.h>
using tasking::Process;
#include <tasking/utilities.h>
#include <execution/RunnableExecutor.h>
#include <execution/ThreadPoolExecutor.h>


int _testLaunch()
{
	int result = 0;
	auto th2 = GenericThread::createEmptyThread(true);	

	th2->fireAndForget(
		[]()
		{
			auto th1 = GenericThread::createEmptyThread(false);			
			//---- tests executors			
			spdlog::info("Launching first task block in a single thread (Executor<Runnable>)");
			
			execution::Executor<Runnable> ex(th1);
			auto cont = ex.launch<int>(  
				[](const auto& v)
				{					
					if ( v.isValid() )
						//spdlog::debug("Value = {}",v.value());
						spdlog::debug("Value ");
					else
						spdlog::error("Error = {}",v.error().errorMsg);
					::tasking::Process::wait(4000);
					// throw std::runtime_error("Error mierdoso");
					
					return 5;
				}
			).next<void>([](const auto& v)
			{
				if ( v.isValid() )
				{
					spdlog::debug("Value = {}",v.value());
				}
				else
				{
					spdlog::error("Error = {}",v.error().errorMsg);
					throw std::runtime_error("Otro error");
				}
			}).next<void>(
				[](const auto& v)
				{
					if ( v.isValid() )
					{
						spdlog::debug("Value");
					}
					else
					{
						spdlog::error("Error = {}",v.error().errorMsg);
					}	
					throw std::runtime_error("Otro error");
				}
			);
			auto r = cont.getResult();			
			spdlog::debug("Waiting for first task block...");			
			th1->start();
			auto wr = tasking::waitForFutureMThread(r);
			if ( wr.isValid() )
				spdlog::debug("First tasks completed successfully");
			else
				spdlog::debug("First tasks completed with error {}",wr.error().errorMsg); 


			spdlog::info("Launching second task block in a thread pool (Executor<ThreadPool>)");
			parallelism::ThreadPool::ThreadPoolOpts opts;
			auto myPool = make_shared<parallelism::ThreadPool>(opts);
			parallelism::ThreadPool::ExecutionOpts exopts;
			execution::Executor<parallelism::ThreadPool> ex2(myPool);
			auto r2 = ex2.launch<int>(  
				[](const auto& v)
				{
					if ( v.isValid() )
						spdlog::debug("Value = {}",v.value());
					else
						spdlog::error("Error = {}",v.error().errorMsg);
					::tasking::Process::wait(2000);
					throw std::runtime_error("Error mierdoso"); //pruebas lanzamiento excepciones
					return 5;
				},9.6f
			).next<string>([](const core::FutureValue<int,::core::ErrorInfo>& v) //can use auto, but for documentation purposes
			{
				if ( v.isValid() )
				{
					spdlog::debug("Value = {}",v.value());
					return "pepe";	
				}
				else
				{
					spdlog::error("Error = {}",v.error().errorMsg);
						throw std::runtime_error("Error final");
				}
			}).next<int>([](const auto& v)
			{
				if ( v.isValid() )
					spdlog::debug("Value = {}",v.value());
				else
					spdlog::error("Error = {}",v.error().errorMsg);
				return 1;
			}).getResult();
			spdlog::debug("Waiting for second task block...");			
			auto wr2 = tasking::waitForFutureMThread(r2);
			if ( wr2.isValid() )
				spdlog::debug("Second task block completed successfully");
			else
				spdlog::debug("Second task block completed with error {}",wr2.error().errorMsg);  
			
		}
	);				
	spdlog::debug("Termino");	
	Thread::sleep(60000);
	spdlog::debug("finish");
	th2->finish();
	th2->join();
	
	return result;
}
int _testFor()
{
	int result = 0;
	Timer timer;
	#define loopSize 100000
	spdlog::set_level(spdlog::level::info); // Set global log level
	auto th1 = GenericThread::createEmptyThread(true);
	execution::Executor<Runnable> ex(th1);	
	spdlog::info("Doing {} iterations using Runnable executor with independent tasks",loopSize);	
	execution::LoopHints lhints{true};

	int mean = 0;
	constexpr size_t iters = 100;
	for(size_t i = 0; i < iters;i++)
	{
		auto t0 = timer.getMilliseconds();
		auto barrier = ex.loop(0,loopSize,
			[](int idx)
			{								
				spdlog::debug("iteracion {}",idx);
				//::tasking::Process::wait(10);
			},lhints,1
		);
		auto elapsed = timer.getMilliseconds()-t0;
		mean += elapsed;
		//th1->resume();
		::core::waitForBarrierThread(barrier);
		//th1->suspend();
	}
	mean /= iters;

	spdlog::info("Finished. Time: {}",mean);

/*en vista de los resutlados de velocidad, me convendría:
 *(LO MAS)- si no necesito el hilo automaticamente arrando, me convendría arrancarlo después de insertar tareas, ya que la parte de la varaible de condicion hace que se note mucho
 - es muy costoso también la creación del PRcoess 
 - la insercion en planificadpr también añade algo pero no tanto
 - el quitar el "virtualismo" en onpostTask no es importante
 */

	
	/*spdlog::info("Doing {} iterations using Runnable executor without indepent tasks",loopSize);	
	t0 = timer.getMilliseconds();
	lhints.independentTasks = false;
	barrier = ex.loop(0,loopSize,
		[](int idx)
		{								
			spdlog::debug("iteracion {}",idx);
			//::tasking::Process::wait(10);
		},lhints,1
	);
	::core::waitForBarrierThread(barrier);
	spdlog::info("Finished. Time: {}",timer.getMilliseconds()-t0);
*/
	return 0;
	spdlog::info("Doing {} iterations using ThreadPool executor",loopSize);
	parallelism::ThreadPool::ThreadPoolOpts opts;
//	opts.nThreads = 1;
	auto myPool = make_shared<parallelism::ThreadPool>(opts);
	auto t0 = timer.getMilliseconds();
	execution::Executor<parallelism::ThreadPool> ex2(myPool);		
	std::atomic<int> count(0);
	auto barrier = ex2.loop(0,loopSize,
		[](int idx)
		{								
			spdlog::debug("iteracion {}",idx);
		//	::tasking::Process::wait(10);
			//++count; //para asegurar
		},lhints
	);
	::core::waitForBarrierThread(barrier);
	spdlog::info("Finished. Time: {}. Count = {}",timer.getMilliseconds()-t0,count);
	spdlog::debug("Termino");	
	Thread::sleep(60000);
	spdlog::debug("finish");
	th1->finish();
	th1->join();
	return result;
}
/**
 * @brief execution tests
 * commandline options
????
 * @return int 
 */
static int test()
{

	int result = 1;
	TestManager::TestType defaultTest = _testLaunch;
	auto opt = tests::CommandLine::getSingleton().getOption("n");
	if ( opt != nullopt)
	{
		try
		{
			auto n = std::stol(opt.value());
			switch(n)
			{
				case 0:
					result = _testLaunch();
					break;
				case 1:
					result = _testFor();
					break;				
				default:;					
			}
		}
		catch(const std::exception& e)
		{
			spdlog::error(e.what());
		}		
	}else
		result = defaultTest(); //by default
		
	return result;
}
void test_execution::registerTest()
{
    TestManager::getSingleton().registerTest(TEST_NAME,"execution tests:\n - 0 = mono thread;\n - 1 = performance launching a bunch of tasks",test);
}
