#include "test.h"
#include <core/ThreadRunnable.h>
using core::ThreadRunnable;
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
	auto th2 = ThreadRunnable::create(true);	

	th2->fireAndForget(
		[]()
		{
			auto th1 = ThreadRunnable::create(false);			
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
template <class F> void _measureTest(string text,F f)
{
	#ifdef NDEBUG
		#define loopSize 1000'000
	#else
		#define loopSize 200'000
	#endif
	int mean = 0;
	constexpr size_t iters = 20;
	Timer timer;
	spdlog::info("Doing {} iterations using {}",loopSize,text);	
	for(size_t i = 0; i < iters;i++)
	{
		auto t0 = timer.getMilliseconds();
		f();
		auto elapsed = timer.getMilliseconds()-t0;
		mean += elapsed;
		//th1->resume();
		
		//th1->suspend();
	}
	mean /= iters;

	spdlog::info("Finished. Time: {}",mean);
}
int _testFor()
{
	int result = 0;
	
	
	spdlog::set_level(spdlog::level::info); // Set global log level
	#define CHUNK_SIZE 512
	
	
	auto th1 = ThreadRunnable::create(false,CHUNK_SIZE);
	_measureTest("Runnable executor with independent tasks and lockOnce",
		[th1]() 
		{	
			//la propia creacion del hilo es ligeramente lenta, y mientras más buffer de tareas, más tarda								
			th1->resume();
			execution::Executor<Runnable> ex(th1);	
			execution::RunnableExecutorLoopHints lhints;
			lhints.independentTasks = true;
			lhints.lockOnce = true;
			auto barrier = ex.loop(0,loopSize,
			[](int idx)
			{								
				spdlog::debug("iteracion {}",idx);
				//::tasking::Process::wait(10);
			},lhints,1
			);
			::core::waitForBarrierThread(barrier);
			spdlog::debug("hecho");
			#ifdef PROCESSSCHEDULER_USE_LOCK_FREE
				th1->getScheduler().resetPool();
			#endif
			th1->suspend();
		}
	);
	
	_measureTest("Runnable executor with independent tasks and NO lockOnce on post",
		[]() 
		{
		
			auto th1 = ThreadRunnable::create(true,CHUNK_SIZE);
			execution::Executor<Runnable> ex(th1);	
			execution::RunnableExecutorLoopHints lhints;
			lhints.independentTasks = true;
			lhints.lockOnce = false;
			auto barrier = ex.loop(0,loopSize,
			[](int idx)
			{								
				spdlog::debug("iteracion {}",idx);
				//::tasking::Process::wait(10);
			},lhints,1
			);
			::core::waitForBarrierThread(barrier);
			spdlog::debug("hecho");
		}
	);	
	_measureTest("Runnable executor with independent tasks,lockOnce and pausing thread",
		[]() mutable
		{
			auto th1 = ThreadRunnable::create(false,CHUNK_SIZE);
			execution::Executor<Runnable> ex(th1);	
			execution::RunnableExecutorLoopHints lhints;
			lhints.independentTasks = true;
			lhints.lockOnce = true;
			auto barrier = ex.loop(0,loopSize,
			[](int idx)
			{								
				spdlog::debug("iteracion {}",idx);
				//::tasking::Process::wait(10);
			},lhints,1
			);
			th1->resume();
			::core::waitForBarrierThread(barrier);
			th1->suspend();
		}
	);
	_measureTest("Runnable executor with independent tasks,NO lockOnce on post and pausing thread",
		[]() mutable
		{
			auto th1 = ThreadRunnable::create(false,CHUNK_SIZE);
			execution::Executor<Runnable> ex(th1);	
			execution::RunnableExecutorLoopHints lhints;
			lhints.independentTasks = true;
			lhints.lockOnce = false;
			auto barrier = ex.loop(0,loopSize,
			[](int idx)
			{								
				spdlog::debug("iteracion {}",idx);
				//::tasking::Process::wait(10);
			},lhints,1
			);
			th1->resume();
			::core::waitForBarrierThread(barrier);
			th1->suspend();
		}
	);
	_measureTest("Runnable executor WITHOUT independent tasks",
		[]()
		{
			auto th1 = ThreadRunnable::create(true,CHUNK_SIZE);
			execution::Executor<Runnable> ex(th1);	
			execution::RunnableExecutorLoopHints lhints;
			lhints.independentTasks = false;
			lhints.lockOnce = true;
			auto barrier = ex.loop(0,loopSize,
			[](int idx)
			{								
				spdlog::debug("iteracion {}",idx);
				//::tasking::Process::wait(10);
			},lhints,1
			);
			::core::waitForBarrierThread(barrier);
		}
	);
	//parallelism::ThreadPool::ThreadPoolOpts opts;
	//auto myPool = make_shared<parallelism::ThreadPool>(opts);
	_measureTest("ThreadPool executor, grouped tasks",
		[]()
		{
			parallelism::ThreadPool::ThreadPoolOpts opts;
			auto myPool = make_shared<parallelism::ThreadPool>(opts);
			execution::Executor<parallelism::ThreadPool> ex2(myPool);
			execution::LoopHints lhints;
			lhints.independentTasks = false;
			auto barrier = ex2.loop(0,loopSize,
			[](int idx)
			{								
				spdlog::debug("iteracion {}",idx);
			//	::tasking::Process::wait(10);
				//++count; //para asegurar
			},lhints);
			::core::waitForBarrierThread(barrier);
		}
	);
	_measureTest("ThreadPool executor, no grouped tasks",
		[]()
		{
			parallelism::ThreadPool::ThreadPoolOpts opts;
			auto myPool = make_shared<parallelism::ThreadPool>(opts);
			execution::Executor<parallelism::ThreadPool> ex2(myPool);
			execution::LoopHints lhints;
			lhints.independentTasks = true;
			auto barrier = ex2.loop(0,loopSize,
			[](int idx)
			{								
				spdlog::debug("iteracion {}",idx);
			//	::tasking::Process::wait(10);
				//++count; //para asegurar
			},lhints);
			::core::waitForBarrierThread(barrier);
		}
	);
	return 0;
	/*
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
	*/
	spdlog::debug("Termino");	
	Thread::sleep(60000);
	spdlog::debug("finish");
	// th1->finish();
	// th1->join();
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
