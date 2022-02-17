#include "test.h"
#include <core/ThreadRunnable.h>
using core::ThreadRunnable;
using namespace std;
#include <TestManager.h>
using tests::TestManager;
#ifdef USE_SPDLOG
#include <spdlog/spdlog.h>
		#endif
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
			#ifdef USE_SPDLOG
			spdlog::info("Launching first task block in a single thread (Executor<Runnable>)");
		#endif
			
			execution::Executor<Runnable> ex(th1);
			auto cont = ex.launch<int>(  
				[](const auto& v)
				{		
					#ifdef USE_SPDLOG
					if ( v.isValid() )
						//spdlog::debug("Value = {}",v.value());
						spdlog::debug("Value ");
					else
						spdlog::error("Error = {}",v.error().errorMsg);
		#endif			
					::tasking::Process::wait(4000);
					// throw std::runtime_error("Error mierdoso");
					
					return 5;
				}
			).next<void>([](const auto& v)
			{
				
				if ( v.isValid() )
				{
				#ifdef USE_SPDLOG
					spdlog::debug("Value = {}",v.value());
				#endif
				}
				else
				{
					#ifdef USE_SPDLOG
					spdlog::error("Error = {}",v.error().errorMsg);
					#endif
					throw std::runtime_error("Otro error");
				}
			}).next<void>(
				[](const auto& v)
				{
					if ( v.isValid() )
					{
						#ifdef USE_SPDLOG
						spdlog::debug("Value");
		#endif
					}
					else
					{
						#ifdef USE_SPDLOG
						spdlog::error("Error = {}",v.error().errorMsg);
		#endif
					}	
					throw std::runtime_error("Otro error");
				}
			);
			auto r = cont.getResult();			
			#ifdef USE_SPDLOG
			spdlog::debug("Waiting for first task block...");			
		#endif
			th1->start();
			auto wr = tasking::waitForFutureMThread(r);
			#ifdef USE_SPDLOG
			if ( wr.isValid() )
				spdlog::debug("First tasks completed successfully");
			else
				spdlog::debug("First tasks completed with error {}",wr.error().errorMsg); 
		#endif

#ifdef USE_SPDLOG
			spdlog::info("Launching second task block in a thread pool (Executor<ThreadPool>)");
		#endif
			parallelism::ThreadPool::ThreadPoolOpts opts;
			auto myPool = make_shared<parallelism::ThreadPool>(opts);
			parallelism::ThreadPool::ExecutionOpts exopts;
			execution::Executor<parallelism::ThreadPool> ex2(myPool);
			auto r2 = ex2.launch<int>(  
				[](const auto& v)
				{
					#ifdef USE_SPDLOG
					if ( v.isValid() )
						spdlog::debug("Value = {}",v.value());
					else
						spdlog::error("Error = {}",v.error().errorMsg);
		#endif
					::tasking::Process::wait(2000);
					throw std::runtime_error("Error mierdoso"); //pruebas lanzamiento excepciones
					return 5;
				},9.6f
			).next<string>([](const core::FutureValue<int,::core::ErrorInfo>& v) //can use auto, but for documentation purposes
			{
				if ( v.isValid() )
				{
					#ifdef USE_SPDLOG
					spdlog::debug("Value = {}",v.value());
		#endif
					return "pepe";	
				}
				else
				{
					#ifdef USE_SPDLOG
					spdlog::error("Error = {}",v.error().errorMsg);
		#endif
					throw std::runtime_error("Error final");
				}
			}).next<int>([](const auto& v)
			{
				#ifdef USE_SPDLOG
				if ( v.isValid() )
					spdlog::debug("Value = {}",v.value());
				else
					spdlog::error("Error = {}",v.error().errorMsg);
		#endif
				return 1;
			}).getResult();
			#ifdef USE_SPDLOG
			spdlog::debug("Waiting for second task block...");			
		#endif
			auto wr2 = tasking::waitForFutureMThread(r2);
			#ifdef USE_SPDLOG
			if ( wr2.isValid() )
				spdlog::debug("Second task block completed successfully");
			else
				spdlog::debug("Second task block completed with error {}",wr2.error().errorMsg);  
		#endif
			
		}
	);				
	Thread::sleep(60000);
	th2->finish();
	th2->join();
	
	return result;
}
#ifdef NDEBUG
#define DEFAULT_LOOPSIZE 1000'000
#else
#define DEFAULT_LOOPSIZE 200'000
#endif
template <class F> void _measureTest(string text,F f, size_t loopSize)
{
	int mean = 0;
	constexpr size_t iters = 20;
	Timer timer;
	#ifdef USE_SPDLOG
	spdlog::info("Doing {} iterations using {}",loopSize,text);	
		#endif
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

#ifdef USE_SPDLOG
	spdlog::info("Finished. Time: {}",mean);
		#endif
}
int _testFor()
{
	int result = 0;
	#ifdef USE_SPDLOG
	spdlog::set_level(spdlog::level::info); // Set global log level

    #endif

    int loopSize = DEFAULT_LOOPSIZE;
    //check for loopsize options, if given
	auto opt = tests::CommandLine::getSingleton().getOption("ls");
	if ( opt != nullopt) {
		loopSize = std::stol(opt.value());
	}
	#define CHUNK_SIZE 512

	auto th1 = ThreadRunnable::create(false,CHUNK_SIZE);
	_measureTest("Runnable executor with independent tasks and lockOnce",
		[th1,loopSize]()
		{	
			//la propia creacion del hilo es ligeramente lenta, y mientras más buffer de tareas, más tarda								
			th1->resume();
			execution::Executor<Runnable> ex(th1);	
			execution::RunnableExecutorLoopHints lhints;
			lhints.independentTasks = true;
			lhints.lockOnce = true;
			const int idx0 = 0;
			auto barrier = ex.loop(idx0,loopSize,
			[](int idx)
			{	
				#ifdef USE_SPDLOG
				spdlog::debug("iteracion {}",idx);
		#endif							
				//::tasking::Process::wait(10);
			},lhints,1
			);
			::core::waitForBarrierThread(barrier);
			#ifdef USE_SPDLOG
			spdlog::debug("hecho");
		#endif
			#ifdef PROCESSSCHEDULER_USE_LOCK_FREE
				th1->getScheduler().resetPool();
			#endif
			th1->suspend();
		},loopSize
	);

	_measureTest("Runnable executor with independent tasks and NO lockOnce on post",
		[loopSize]()
		{
			auto th1 = ThreadRunnable::create(true,CHUNK_SIZE);
			execution::Executor<Runnable> ex(th1);	
			execution::RunnableExecutorLoopHints lhints;
			lhints.independentTasks = true;
			lhints.lockOnce = false;
			const int idx0 = 0;
			auto barrier = ex.loop(idx0,loopSize,
			[](int idx)
			{	
				#ifdef USE_SPDLOG
				spdlog::debug("iteracion {}",idx);
		#endif							
				//::tasking::Process::wait(10);
			},lhints,1
			);
			::core::waitForBarrierThread(barrier);
		},loopSize
	);

	_measureTest("Runnable executor with independent tasks,lockOnce and pausing thread",
		[loopSize]() mutable
		{
			auto th1 = ThreadRunnable::create(false,CHUNK_SIZE);
			execution::Executor<Runnable> ex(th1);	
			execution::RunnableExecutorLoopHints lhints;
			lhints.independentTasks = true;
			lhints.lockOnce = true;
			int idx0 = 0;
			auto barrier = ex.loop(idx0,loopSize,
			[](int idx)
			{	
				#ifdef USE_SPDLOG
				spdlog::debug("iteracion {}",idx);
		#endif							
				//::tasking::Process::wait(10);
			},lhints,1
			);
			th1->resume();
			::core::waitForBarrierThread(barrier);
			th1->suspend();
		},loopSize
	);
	_measureTest("Runnable executor with independent tasks,NO lockOnce on post and pausing thread",
		[loopSize]() 	
		{
			auto th1 = ThreadRunnable::create(false,CHUNK_SIZE);
			execution::Executor<Runnable> ex(th1);	
			execution::RunnableExecutorLoopHints lhints;
			lhints.independentTasks = true;
			lhints.lockOnce = false;
			const int idx0 = 0;
			auto barrier = ex.loop(idx0,loopSize,
			[](int idx)
			{	
				#ifdef USE_SPDLOG
				spdlog::debug("iteracion {}",idx);
		#endif							
				//::tasking::Process::wait(10);
			},lhints,1
			);
			th1->resume();
			::core::waitForBarrierThread(barrier);
			th1->suspend();
		},loopSize
	);
	_measureTest("Runnable executor WITHOUT independent tasks",
		[loopSize]()
		{
			auto th1 = ThreadRunnable::create(true,CHUNK_SIZE);
			execution::Executor<Runnable> ex(th1);	
			execution::RunnableExecutorLoopHints lhints;
			lhints.independentTasks = false;
			lhints.lockOnce = true;
			const int idx0 = 0;
			auto barrier = ex.loop(idx0,loopSize,
			[](int idx)
			{	
				#ifdef USE_SPDLOG
				spdlog::debug("iteracion {}",idx);
		#endif							
				//::tasking::Process::wait(10);
			},lhints,1
			);
			::core::waitForBarrierThread(barrier);
		},loopSize
	);
	//parallelism::ThreadPool::ThreadPoolOpts opts;
	//auto myPool = make_shared<parallelism::ThreadPool>(opts);
	_measureTest("ThreadPool executor, grouped tasks",
		[loopSize]()
		{
			parallelism::ThreadPool::ThreadPoolOpts opts;
			auto myPool = make_shared<parallelism::ThreadPool>(opts);
			execution::Executor<parallelism::ThreadPool> ex2(myPool);
			execution::LoopHints lhints;
			lhints.independentTasks = false;
			const int idx0 = 0;
			auto barrier = ex2.loop(idx0,loopSize,
			[](int idx)
			{	
				#ifdef USE_SPDLOG
				spdlog::debug("iteracion {}",idx);
		#endif							
			//	::tasking::Process::wait(10);
				//++count; //para asegurar
			},lhints);
			::core::waitForBarrierThread(barrier);
		},loopSize
	);
	_measureTest("ThreadPool executor, no grouped tasks",
		[loopSize]()
		{
			parallelism::ThreadPool::ThreadPoolOpts opts;
			auto myPool = make_shared<parallelism::ThreadPool>(opts);
			execution::Executor<parallelism::ThreadPool> ex2(myPool);
			execution::LoopHints lhints;
			lhints.independentTasks = true;
			const int idx0 = 0;
			auto barrier = ex2.loop(idx0,loopSize,
			[](int idx)
			{	
				#ifdef USE_SPDLOG
				spdlog::debug("iteracion {}",idx);
		#endif							
			//	::tasking::Process::wait(10);
				//++count; //para asegurar
			},lhints);
			::core::waitForBarrierThread(barrier);
		},loopSize
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
	
	Thread::sleep(60000);
	
	// th1->finish();
	// th1->join();
	return result;
}
/**
 * @brief execution tests
 * commandline options
	-n: test number (0->test launch; 1->test for)
  	-ls: loop size for test number 1
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
				default:
					spdlog::error("Test number {} doesn't exist",n);
			}
		}
		catch(const std::exception& e)
		{
			#ifdef USE_SPDLOG
			spdlog::error(e.what());
		#endif
		}		
	}else
		result = defaultTest(); //by default
		
	return result;
}
void test_execution::registerTest()
{
    TestManager::getSingleton().registerTest(TEST_NAME,"execution tests:\n - 0 = mono thread;\n - 1 = performance launching a bunch of tasks",test);
}
