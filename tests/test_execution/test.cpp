#include "test.h"
using test_execution::TestExecution;
#include <core/ThreadRunnable.h>
using core::ThreadRunnable;
using namespace std;
#include <TestManager.h>
using tests::TestManager;
#include <text/logger.h>
#include <CommandLine.h>
#include <mpl/LinkArgs.h>
#include <mpl/Ref.h>
#include <string>
#include <tasking/Process.h>
using tasking::Process;
#include <tasking/utilities.h>
#include <execution/RunnableExecutor.h>
#include <execution/ThreadPoolExecutor.h>

const std::string TestExecution::TEST_NAME = "execution";
//b aseictest for launching task in executio agents
int _testLaunch( tests::BaseTest* test)
{
	int result = 0;		
	{
		auto th1 = ThreadRunnable::create(true);	
		auto th2 = ThreadRunnable::create(true);	
		execution::Executor<Runnable> ex(th1);

		auto fut = execution::launch<int>(ex,
					[]()
					{					
						text::info("Current Runnable {}",static_cast<void*>(ThreadRunnable::getCurrentRunnable()));
						text::info("Launch waiting");
						::tasking::Process::wait(5000);
						text::info("Launch done");
						return 4;
					}
				);
		//Thread::sleep(10000);
		execution::Executor<Runnable> ex2(th2);
		//::core::waitForFutureThread(fut);	
		//auto fut_1 = execution::transfer(fut,ex2);
		fut = execution::transfer(fut,ex2);
		auto fut2 = execution::next<void>(fut,[](const auto& v)
				{
					text::info("Current Runnable {}",static_cast<void*>(ThreadRunnable::getCurrentRunnable()));
					if (v.isValid())
					{
						text::info("Next done: {}",v.value());		
					}else					
						text::error("Next error: {}",v.error().errorMsg);		
				}
				);			
		::core::waitForFutureThread(fut2);	
		/*auto fut3 = execution::launch<void>(ex,
			[]()
			{					
				text::info("Launch 2 waiting");
				::tasking::Process::wait(5000);
				text::info("Launch 2 done");
			}
		);
		auto fut4 = execution::next<int>(ex,fut3,[](const auto& v)
				{
					if (v.isValid())
					{
						text::info("Next 2 done");
					}else					
						text::error("Next 2 error: {}",v.error().errorMsg);		
						return 7;
				}
				);	
		auto fut5 = execution::next<void>(ex,fut4,[](const auto& v)
				{
					if (v.isValid())
					{
						text::info("Next 3 done {}",v.value());		
					}else					
						text::error("Next 3 error: {}",v.error().errorMsg);		
				}
				);
				*/
		//@todo retomar el tema de los hints y seguir con threadpoolexecutor
		/*auto cont = execution::launch<int>(ex,
					[](const auto& v)
					{					
						if (v.isValid())
						{
							text::info("Launch done: {}",v.value());		
						}else					
							text::error("Launch error: {}",v.error().errorMsg);		
						if ( ::tasking::Process::wait(5000) == ::tasking::Process::ESwitchResult::ESWITCH_KILL )
							text::error("Launch killed!");
						return 4;
					},5.6f
				)			
				.next<void>( [](const auto& v)
				{
					if (v.isValid())
					{
						text::info("Next done: {}",v.value());		
					}else					
						text::error("Next error: {}",v.error().errorMsg);		
				}
				)
				;
				*/
		//::core::waitForFutureThread(fut5);
	}
	{		
		parallelism::ThreadPool::ThreadPoolOpts opts;
		auto myPool = make_shared<parallelism::ThreadPool>(opts);
		parallelism::ThreadPool::ExecutionOpts exopts;
		execution::Executor<parallelism::ThreadPool> ex(myPool);
		auto fut = execution::launch<int>(ex,
					[](float v)
					{					
						text::info("Tp Launch waiting");
						::tasking::Process::wait(5000);
						text::info("TP Launch done: {}",v);
						return 4;
					},5.6f
				);
		Thread::sleep(10000);
		auto fut2 = execution::next<void>(fut,[](const auto& v)
				{
					if (v.isValid())
					{
						text::info("Next done: {}",v.value());		
					}else					
						text::error("Next error: {}",v.error().errorMsg);		
				}
				);		
		::core::waitForFutureThread(fut2);                

	}
	text::info("TP Sleep");
	Thread::sleep(5000);
	return 0;
/*
auto th2 = ThreadRunnable::create(true);	
	th2->fireAndForget(
		[]()
		{
			auto th1 = ThreadRunnable::create(false);			
			//---- tests executors						
			text::info("Launching first task block in a single thread (Executor<Runnable>)");
			
			execution::Executor<Runnable> ex(th1);
			
			auto cont = ex.launch<int>(  
				[](const auto& v) //int this case,no argument given, so v is void
				{							
					if ( v.isValid() )
						//spdlog::debug("Value = {}",v.value());
						text::debug("Value");
					else
						text::error("Error = {}",v.error().errorMsg);
		
					::tasking::Process::wait(4000);
					// throw std::runtime_error("Error mierdoso");
					
					return 5;
				}
			).next<void>([](const auto& v)
			{
				
				if ( v.isValid() )
				{
					text::debug("Value = {}",v.value());
				}
				else
				{
					text::error("Error = {}",v.error().errorMsg);
					throw std::runtime_error("Otro error");
				}
			}).next<void>(
				[](const auto& v)
				{
					if ( v.isValid() )
					{						
						text::debug("Value");
					}
					else
					{						
						text::error("Error = {}",v.error().errorMsg);
					}	
					throw std::runtime_error("Otro error");
				}
			);
			auto r = cont.getResult();			
			text::debug("Waiting for first task block...");			
			th1->resume();
			auto wr = tasking::waitForFutureMThread(r);

			if ( wr.isValid() )
				text::debug("First tasks completed successfully");
			else
				text::debug("First tasks completed with error {}",wr.error().errorMsg); 

			text::info("Launching second task block in a thread pool (Executor<ThreadPool>)");

			parallelism::ThreadPool::ThreadPoolOpts opts;
			auto myPool = make_shared<parallelism::ThreadPool>(opts);
			parallelism::ThreadPool::ExecutionOpts exopts;
			execution::Executor<parallelism::ThreadPool> ex2(myPool);
			auto r2 = ex2.launch<int>(  
				[](const auto& v)
				{					
					if ( v.isValid() )
						text::debug("Value = {}",v.value());
					else
						text::error("Error = {}",v.error().errorMsg);
					::tasking::Process::wait(2000);
					throw std::runtime_error("Error mierdoso"); //pruebas lanzamiento excepciones
					return 5;
				},9.6f
			).next<string>([](const core::FutureValue<int,::core::ErrorInfo>& v) //can use auto, but for documentation purposes
			{
				if ( v.isValid() )
				{					
					text::debug("Value = {}",v.value());
					return "pepe";	
				}
				else
				{
					text::error("Error = {}",v.error().errorMsg);
					throw std::runtime_error("Error final");
				}
			}).next<int>([](const auto& v)
			{			
				if ( v.isValid() )
					text::debug("Value = {}",v.value());
				else
					text::error("Error = {}",v.error().errorMsg);
				return 1;
			}).getResult();
			
			text::debug("Waiting for second task block...");			
		
			auto wr2 = tasking::waitForFutureMThread(r2);
			if ( wr2.isValid() )
				text::debug("Second task block completed successfully");
			else
				text::debug("Second task block completed with error {}",wr2.error().errorMsg);  
			
		}
	);				
Thread::sleep(10000);
*/	
	return result;
}
#ifdef NDEBUG
#define DEFAULT_LOOPSIZE 500'000
#else
#define DEFAULT_LOOPSIZE 100'000
#endif
template <class F> void _measureTest(string txt,F f, size_t loopSize)
{
	int mean = 0;
	constexpr size_t iters = 20;
	Timer timer;
	text::info("Doing {} iterations using {}",loopSize,txt);	
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

	tests::BaseTest::addMeasurement(txt+" time:",mean);
	text::info("Finished. Time: {}",mean);
}
auto gTestFunc=[](int idx)
	{
		text::debug("iteracion pre {}",idx);
		::tasking::Process::switchProcess(true);
		text::debug("iteracion post {}",idx);
	};
int _testFor(tests::BaseTest* test)
{
	int result = 0;
	// text::set_level(text::level::info);
	
    // int loopSize = DEFAULT_LOOPSIZE;
    // //check for loopsize options, if given
	// auto opt = tests::CommandLine::getSingleton().getOption("ls");
	// if ( opt != nullopt) {
	// 	loopSize = std::stol(opt.value());
	// }
	// #define CHUNK_SIZE 512	
	
	// _measureTest("Runnable executor with independent tasks and lockOnce",
	// 	[loopSize]()
	// 	{	
	// 		auto th1 = ThreadRunnable::create(true,CHUNK_SIZE);
	// 		execution::Executor<Runnable> ex(th1);			
	// 		execution::RunnableExecutorLoopHints lhints;
	// 		lhints.independentTasks = true;
	// 		lhints.lockOnce = true;
	// 		const int idx0 = 0;
	// 		auto barrier = ex.loop(idx0,loopSize,gTestFunc,lhints,1);
	// 		::core::waitForBarrierThread(barrier);
	// 		text::debug("hecho");
	// 		#ifdef PROCESSSCHEDULER_USE_LOCK_FREE
	// 			th1->getScheduler().resetPool();
	// 		#endif
	// 	},loopSize
	// );

	// _measureTest("Runnable executor with independent tasks and NO lockOnce on post",
	// 	[loopSize]()
	// 	{
	// 		auto th1 = ThreadRunnable::create(true,CHUNK_SIZE);
	// 		execution::Executor<Runnable> ex(th1);	
	// 		execution::RunnableExecutorLoopHints lhints;
	// 		lhints.independentTasks = true;
	// 		lhints.lockOnce = false;
	// 		const int idx0 = 0;
	// 		auto barrier = ex.loop(idx0,loopSize,gTestFunc,lhints,1
	// 		);
	// 		::core::waitForBarrierThread(barrier);
	// 	},loopSize
	// );

	// _measureTest("Runnable executor with independent tasks,lockOnce and pausing thread",
	// 	[loopSize]() mutable
	// 	{
	// 		auto th1 = ThreadRunnable::create(false,CHUNK_SIZE);
	// 		execution::Executor<Runnable> ex(th1);	
	// 		execution::RunnableExecutorLoopHints lhints;
	// 		lhints.independentTasks = true;
	// 		lhints.lockOnce = true;
	// 		int idx0 = 0;
	// 		auto barrier = ex.loop(idx0,loopSize,gTestFunc,lhints,1);
	// 		th1->resume();
	// 		::core::waitForBarrierThread(barrier);
	// 		th1->suspend();
	// 	},loopSize
	// );
	// _measureTest("Runnable executor with independent tasks,NO lockOnce on post and pausing thread",
	// 	[loopSize]() 	
	// 	{
	// 		auto th1 = ThreadRunnable::create(false,CHUNK_SIZE);
	// 		execution::Executor<Runnable> ex(th1);	
	// 		execution::RunnableExecutorLoopHints lhints;
	// 		lhints.independentTasks = true;
	// 		lhints.lockOnce = false;
	// 		const int idx0 = 0;
	// 		auto barrier = ex.loop(idx0,loopSize,gTestFunc,lhints,1);
	// 		th1->resume();
	// 		::core::waitForBarrierThread(barrier);
	// 		th1->suspend();
	// 	},loopSize
	// );
	// _measureTest("Runnable executor WITHOUT independent tasks",
	// 	[loopSize]()
	// 	{
	// 		auto th1 = ThreadRunnable::create(true,CHUNK_SIZE);
	// 		execution::Executor<Runnable> ex(th1);	
	// 		execution::RunnableExecutorLoopHints lhints;
	// 		lhints.independentTasks = false;
	// 		lhints.lockOnce = true;
	// 		const int idx0 = 0;
	// 		auto barrier = ex.loop(idx0,loopSize,gTestFunc,lhints,1);
	// 		::core::waitForBarrierThread(barrier);
	// 	},loopSize
	// );
	// //parallelism::ThreadPool::ThreadPoolOpts opts;
	// //auto myPool = make_shared<parallelism::ThreadPool>(opts);
	// _measureTest("ThreadPool executor, grouped tasks",
	// 	[loopSize]()
	// 	{
	// 		parallelism::ThreadPool::ThreadPoolOpts opts;
	// 		auto myPool = make_shared<parallelism::ThreadPool>(opts);
	// 		execution::Executor<parallelism::ThreadPool> ex2(myPool);
	// 		execution::LoopHints lhints;
	// 		lhints.independentTasks = false;
	// 		const int idx0 = 0;
	// 		auto barrier = ex2.loop(idx0,loopSize,gTestFunc,lhints);
	// 		::core::waitForBarrierThread(barrier);
	// 	},loopSize
	// );
	/*
	lo quito por ahora porque es mucho más lento y es bobada
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
				text::debug("iteracion {}",idx);
			//	::tasking::Process::wait(10);
				//++count; //para asegurar
			},lhints);
			::core::waitForBarrierThread(barrier);
		},loopSize
	);*/
	
	return 0;
	
}
/**
 * @brief execution tests
 * commandline options
	-n: test number (0->test launch; 1->test for)
  	-ls: loop size for test number 1
 * @return int 
 */
int TestExecution::onExecuteTest()
{
	int result = 1;
	typedef int(*TestType)(tests::BaseTest* );
	TestType defaultTest = _testFor;
	auto opt = tests::CommandLine::getSingleton().getOption("n");
	if ( opt != nullopt)
	{
		try
		{
			auto n = std::stol(opt.value());
			switch(n)
			{
				case 0:
					result = _testLaunch(this);
					break;
				case 1:
					result = _testFor(this);
					break;				
				default:
				#if USE_SPDLOG
					spdlog::warn("Test number {} doesn't exist. Executing default test",n);
		#endif
					result = _testFor(this);

			}
		}
		catch(const std::exception& e)
		{
			text::error(e.what());
		}		
	}else
		result = defaultTest(this); //by default
		
	return result;
}
void TestExecution::registerTest()
{
    TestManager::getSingleton().registerTest(TEST_NAME,"execution tests:\n - 0 = mono thread;\n - 1 = performance launching a bunch of tasks",std::make_unique<TestExecution>());
}
int TestExecution::onExecuteAllTests()
{
	_testLaunch(this);
	_testFor(this);
	return 0;
}