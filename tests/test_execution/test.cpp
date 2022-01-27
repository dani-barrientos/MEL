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

/**
 * @brief execution tests
 * commandline options
????
 * @return int 
 */
static int test()
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
			tasking::waitForFutureMThread(r);
			if ( r.getValid() )
				spdlog::debug("First tasks completed successfully");
			else
				spdlog::debug("First tasks completed with error {}",r.getValue().error().errorMsg); 
		
			spdlog::info("Launching second task block in a thread pool (Executor<ThreadPool>)");


			parallelism::ThreadPool::ThreadPoolOpts opts;
			auto myPool = make_shared<parallelism::ThreadPool>(opts);
			parallelism::ThreadPool::ExecutionOpts exopts;
			execution::Executor<parallelism::ThreadPool> ex2(myPool);
			auto r2 = ex.launch<int>(  
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
			tasking::waitForFutureMThread(r2);
			if ( r2.getValid() )
				spdlog::debug("Second task block completed successfully");
			else
				spdlog::debug("Second task block completed with error {}",r2.getValue().error().errorMsg);  
			
		}
	);	
	// ¿qué hago para coger/esperar por el resultado final? POSIBILIDADES:
	// 	- USAR UN NEXT QUE HARÁ LO QUE SE QUIERE->ES LO MÁS DIRECTO, NO REQUIERE CAMBIOS->EL PROBLEMA ES QUE ESO SE EJECUTA EN EL RUNNABLE DESTINO
	// 	- TENER UN WAIT EN EL CONTINUATION
	// 	- TENER UN FUTURE CON EL RESULTADO->CREO QUE SERÁ LO MEJOR, PERO IMPLICA OTRO MIEMBRO MÁS EN EL CONTINAUTION
		
	spdlog::debug("Termino");
	//¿deberái poder coger el resultado final? Es decir, de alguna forma que me dé el 
	
/*
{
	Executor<Runnable> ex(th1);
	auto cont = ex.launch(
		[]()
		{
			spdlog::debug("UNO");
			::tasking::Process::wait(4000);
			spdlog::debug("DOS");
		}
	).next([]()
		{
			spdlog::debug("TRES");
			::tasking::Process::wait(1000);
			spdlog::debug("CUATRO");
		}
	);
//	spdlog::debug("Remaining Data 1: {}",ContinuationData::counter);
	Thread::sleep(3000);
//	spdlog::debug("Remaining Data 2: {}",ContinuationData::counter);
	cont.next([]()
		{
			spdlog::debug("CINCO");
		}
	);	
	}
	*/
//	spdlog::debug("Remaining Data 3: {}",ContinuationData::counter);	
	/*Executor<Runnable> ex(th1);
	auto cont = ex.launch(
		[]()
		{
			return 5;
		}
	);
	*/
	Thread::sleep(60000);
	spdlog::debug("finish");
	th2->finish();
	th2->join();
	
	return result;
}
void test_execution::registerTest()
{
    TestManager::getSingleton().registerTest(TEST_NAME,"execution tests:\n - 0 = mono thread;\n - 1 = performance launching a bunch of tasks",test);
}
