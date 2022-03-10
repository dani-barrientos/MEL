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

//test for development purposes
int _testDebug()
{
int result = 0;		
	{
		
		auto th1 = ThreadRunnable::create(true);	
		auto th2 = ThreadRunnable::create(true);	
		execution::Executor<Runnable> ex(th1);		
		ex.setOpts({true,false,false});
		
		//----		
		{
		const int idx0 = 0;
		const int loopSize = 10;
		auto kk1 = execution::launch(ex,
			[]()
			{		
				text::info("Launch kk");			
				::tasking::Process::wait(5000);					
				throw std::runtime_error("Error1");
				return std::make_tuple(6,7);
			}
		);
		
		auto kk0 = execution::bulk(execution::inmediate(execution::start(ex),"hola"s),[](const auto& v)
		{
			text::info("Runnable Bulk 1");
			::tasking::Process::wait(2000);
			if ( v.isValid() )	
				text::info("Bulk 1 Value = {}",v.value());
			else
				text::info("Bulk 1 Error = {}",v.error().errorMsg);
		},[](const auto& v)
		{
			text::info("Runnable Bulk 2");		
			::tasking::Process::wait(3000);	
			if ( v.isValid() )	
				text::info("Bulk 2 Value = {}",v.value());
			else
				text::info("Bulk 2 Error = {}",v.error().errorMsg);
		},[](const auto& v)
		{
			text::info("Runnable Bulk 3");		
			
			if ( v.isValid() )	
				text::info("Bulk 3 Value = {}",v.value());
			else
				text::info("Bulk 3 Error = {}",v.error().errorMsg);
		}
		);	
		core::waitForFutureThread(kk0);
		auto kk2 = execution::loop(kk1,idx0,loopSize,
			[](int idx,const auto& v)
			{
				::tasking::Process::wait(1000);
				if ( v.isValid() )
				{
					const auto& val = v.value();
					text::info("It {}. Value = {}",idx,std::get<0>(val));				
				}
				else
					text::info("It {}. Error = {}",idx,v.error().errorMsg);				
				
				

			}
		);
		auto kk3 = execution::next(kk2,[](const auto& v)->int
		{								
			text::info("Launch waiting");
			if ( ::tasking::Process::wait(5000) != tasking::Process::ESwitchResult::ESWITCH_KILL )
			{
				//throw std::runtime_error("Error1");
				text::info("Launch done");
			}else
				text::info("Launch killed");
			return 4;
		}
		);
		::core::waitForFutureThread(kk3);
		text::info("Done!!");
		}
		//---
		auto fut = execution::next(execution::start(ex),
					[](const auto& v)->int
					{					
						text::info("Current Runnable {}",static_cast<void*>(ThreadRunnable::getCurrentRunnable()));
						text::info("Launch waiting");
						if ( ::tasking::Process::wait(5000) != tasking::Process::ESwitchResult::ESWITCH_KILL )
						{
							//throw std::runtime_error("Error1");
							text::info("Launch done");
						}else
							text::info("Launch killed");
						return 4;
					}
				);
		//Thread::sleep(10000);
		execution::Executor<Runnable> ex2(th2);
		//::core::waitForFutureThread(fut);	
		//auto fut_1 = execution::transfer(fut,ex2);
		fut = execution::transfer(fut,ex2);
		auto fut2 = execution::next(fut,[](const auto& v)
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
	}
	{		
		parallelism::ThreadPool::ThreadPoolOpts opts;
		auto myPool = make_shared<parallelism::ThreadPool>(opts);
		parallelism::ThreadPool::ExecutionOpts exopts;
		execution::Executor<parallelism::ThreadPool> ex(myPool);
		ex.setOpts({true,true});
		/*
		auto kk_0 = execution::launch(ex,
			[]()->const char*
			{
				return "hola";
			}
		);
		@todo por alguna razón si el string siguiente lo paso como const char* (es decir, sin la 's') no me compila,
		algo relativo al FutureValue. Sin embargo haciendo un launch que devuelve const char* sí
		*/
		auto kk0 = execution::bulk(execution::inmediate(execution::start(ex),"hola"s),[](const auto& v)
		{
			text::info("Bulk 1");
			::tasking::Process::wait(2000);
			if ( v.isValid() )	
				text::info("Bulk 1 Value = {}",v.value());
			else
				text::info("Bulk 1 Error = {}",v.error().errorMsg);
		},[v2 = 9.6f](const auto& v)
		{
			text::info("Bulk 2");		
			::tasking::Process::wait(3000);	
			if ( v.isValid() )	
				text::info("Bulk 2 Value = {} and {}",v.value(),v2);
			else
				text::info("Bulk 2 Error = {}",v.error().errorMsg);
		}
		);			
		::core::waitForFutureThread(kk0);
		{
			
		const int idx0 = 0;
		const int loopSize = 10;		
		auto kk1 = execution::launch(ex,
			[]()
			{		
				text::info("Launch TP kk");			
				::tasking::Process::wait(5000);					
				//throw std::runtime_error("Error1");
				return "pepe";
			}
		);
		auto kk = execution::loop(kk1/*execution::schedule(ex)*/,idx0,loopSize,
			[](int idx,const auto& v)
			{
				::tasking::Process::wait(1000);
				if ( v.isValid())
				{
					text::info("It {}. Value = {}",idx,v.value());				
				}else
					text::error("It {} .Err = {}",idx,v.error().errorMsg);				
				
			}
		);
		// text::info("voy a esperar");
		// ::core::waitForFutureThread(kk);
		text::info("terminó");
		}
		auto fut = execution::launch(ex,
			[](string v)
			{					
				text::info("Tp Launch waiting");
				if ( ::tasking::Process::wait(5000) != tasking::Process::ESwitchResult::ESWITCH_KILL )
					text::info("TP Launch done: {}",v);
				else
					text::info("TP Launch killed");
				return 4;
			},"pepe"
		);
		// text::info("TP Sleep");
		// Thread::sleep(10000);
		auto fut2 = execution::next(fut,[](const auto& v)->void
				{
					if (v.isValid())
					{
						text::info("TP Next done: {}",v.value());		
					}else					
						text::error("TP Next error: {}",v.error().errorMsg);		
				}
				);		
		::core::waitForFutureThread(fut2);                

	}	
	Thread::sleep(5000);
	return 0;
}
//basic test for launching task in executio agents
int _testLaunch( tests::BaseTest* test)
{
	int result = 0;		
	{
		auto th1 = ThreadRunnable::create(true);	
		auto th2 = ThreadRunnable::create(true);	
		execution::Executor<Runnable> ex(th1);		
		ex.setOpts({true,false,false});
		
		//----		
		{
		const int idx0 = 0;
		const int loopSize = 10;
		auto kk1 = execution::launch(ex,
			[]()
			{		
				text::info("Launch kk");			
				::tasking::Process::wait(5000);					
				throw std::runtime_error("Error1");
				return "pepe";
			}
		);
		auto kk2 = execution::loop(kk1,idx0,loopSize,
			[](int idx,const auto& v)
			{
				::tasking::Process::wait(1000);
				if ( v.isValid() )
					text::info("It {}. Value = {}",idx,v.value());				
				else
					text::info("It {}. Error = {}",idx,v.error().errorMsg);				

			}
		);
		auto kk3 = execution::next(kk2,[](const auto& v)->int
		{								
			text::info("Launch waiting");
			if ( ::tasking::Process::wait(5000) != tasking::Process::ESwitchResult::ESWITCH_KILL )
			{
				//throw std::runtime_error("Error1");
				text::info("Launch done");
			}else
				text::info("Launch killed");
			return 4;
		}
		);
		::core::waitForFutureThread(kk3);
		text::info("Done!!");
		}
		//---
		auto fut = execution::next(execution::start(ex),
					[](const auto& v)->int
					{					
						text::info("Current Runnable {}",static_cast<void*>(ThreadRunnable::getCurrentRunnable()));
						text::info("Launch waiting");
						if ( ::tasking::Process::wait(5000) != tasking::Process::ESwitchResult::ESWITCH_KILL )
						{
							//throw std::runtime_error("Error1");
							text::info("Launch done");
						}else
							text::info("Launch killed");
						return 4;
					}
				);
		//Thread::sleep(10000);
		execution::Executor<Runnable> ex2(th2);
		//::core::waitForFutureThread(fut);	
		//auto fut_1 = execution::transfer(fut,ex2);
		fut = execution::transfer(fut,ex2);
		auto fut2 = execution::next(fut,[](const auto& v)
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
	}
	{		
		parallelism::ThreadPool::ThreadPoolOpts opts;
		auto myPool = make_shared<parallelism::ThreadPool>(opts);
		parallelism::ThreadPool::ExecutionOpts exopts;
		execution::Executor<parallelism::ThreadPool> ex(myPool);
		ex.setOpts({true,true});
		{
		const int idx0 = 0;
		const int loopSize = 10;		
		auto kk1 = execution::launch(ex,
			[]()
			{		
				text::info("Launch TP kk");			
				::tasking::Process::wait(5000);					
				//throw std::runtime_error("Error1");
				return "pepe";
			}
		);
		// auto kk = execution::loop(kk1/*execution::schedule(ex)*/,idx0,loopSize,
		// 	[](int idx,const auto& v)
		// 	{
		// 		::tasking::Process::wait(1000);
		// 		if ( v.isValid())
		// 		{
		// 			text::info("It {}. Value = {}",idx,v.value());				
		// 		}else
		// 			text::error("It {} .Err = {}",idx,v.error().errorMsg);				
				
		// 	}
		// );
		// text::info("voy a esperar");
		// ::core::waitForFutureThread(kk);
		text::info("terminó");
		}
		auto fut = execution::launch(ex,
			[](string v)
			{					
				text::info("Tp Launch waiting");
				if ( ::tasking::Process::wait(5000) != tasking::Process::ESwitchResult::ESWITCH_KILL )
					text::info("TP Launch done: {}",v);
				else
					text::info("TP Launch killed");
				return 4;
			},"pepe"
		);
		// text::info("TP Sleep");
		// Thread::sleep(10000);
		auto fut2 = execution::next(fut,[](const auto& v)->void
				{
					if (v.isValid())
					{
						text::info("TP Next done: {}",v.value());		
					}else					
						text::error("TP Next error: {}",v.error().errorMsg);		
				}
				);		
		::core::waitForFutureThread(fut2);                

	}	
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
	
    int loopSize = DEFAULT_LOOPSIZE;
    //check for loopsize options, if given
	auto opt = tests::CommandLine::getSingleton().getOption("ls");
	if ( opt != nullopt) {
		loopSize = std::stol(opt.value());
	}
	#define CHUNK_SIZE 512	
	
	// _measureTest("Runnable executor with independent tasks and lockOnce",
	// 	[loopSize]()
	// 	{	
	// 		auto th1 = ThreadRunnable::create(true,CHUNK_SIZE);
	// 		execution::Executor<Runnable> ex(th1);			
	// 		execution::RunnableExecutorOpts opts;
	// 		opts.independentTasks = true;
	// 		opts.lockOnce = true;
	// 		ex.setOpts(opts);
	// 		const int idx0 = 0;
	// 		auto barrier = execution::loop(ex,idx0,loopSize,gTestFunc,1);
	// 		::core::waitForBarrierThread(barrier);
	// 		text::debug("hecho");
	// 		#ifdef PROCESSSCHEDULER_USE_LOCK_FREE
	// 			th1->getScheduler().resetPool();
	// 		#endif
	// 	},loopSize
	// );

	/*_measureTest("Runnable executor with independent tasks and NO lockOnce on post",
		[loopSize]()
		{
			auto th1 = ThreadRunnable::create(true,CHUNK_SIZE);
			execution::Executor<Runnable> ex(th1);	
			execution::RunnableExecutorLoopHints lhints;
			lhints.independentTasks = true;
			lhints.lockOnce = false;
			const int idx0 = 0;
			auto barrier = ex.loop(idx0,loopSize,gTestFunc,lhints,1
			);
			::core::waitForBarrierThread(barrier);
		},loopSize
	);
*/
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
	// 		execution::ThreadPoolExecutorOpts exopts;
	// 		exopts.independentTasks = false;
	// 		ex2.setOpts(exopts);
	// 		const int idx0 = 0;
	// 		auto barrier = execution::loop(ex2,idx0,loopSize,gTestFunc);
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
				case 1000:
					_testDebug();
					break;
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