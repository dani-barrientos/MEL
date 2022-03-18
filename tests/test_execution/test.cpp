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
struct MyErrorInfo : public ::core::ErrorInfo
{
	MyErrorInfo(int code,string msg):ErrorInfo(code,std::move(msg))
	{
		text::debug("MyErrorInfo");
	}
	MyErrorInfo(MyErrorInfo&& ei):ErrorInfo(ei.error,std::move(ei.errorMsg)){}
	MyErrorInfo(const MyErrorInfo& ei):ErrorInfo(ei.error,ei.errorMsg){}
	MyErrorInfo& operator = (MyErrorInfo&& info)
	{
		error = info.error;
		errorMsg = std::move(info.errorMsg);
		return *this;
	}
	MyErrorInfo& operator = (const MyErrorInfo& info)
	{
		error = info.error;
		errorMsg = info.errorMsg;
		return *this;
	}
};
/*
 * class for testing copies, moves, etc..
 * 
 */
struct TestClass
{
	int val;
	tests::BaseTest* test;
	tests::BaseTest::LogLevel logLevel;
	TestClass(tests::BaseTest* aTest,tests::BaseTest::LogLevel ll = tests::BaseTest::LogLevel::Info):val(1),test(aTest),logLevel(ll)
	{
//		text::info("TestClass constructor");
		test->addTextToBuffer("TestClass constructor\n",logLevel);
	}
	explicit TestClass(int aVal,tests::BaseTest* aTest,tests::BaseTest::LogLevel ll = tests::BaseTest::LogLevel::Info):val(aVal),test(aTest),logLevel(ll)
	{
		test->addTextToBuffer("TestClass constructor\n",logLevel);
	}
	TestClass(const TestClass& ob)
	{
		val = ob.val;
		test = ob.test;
		logLevel = ob.logLevel;
		test->addTextToBuffer("TestClass copy constructor\n",logLevel);
	}
	TestClass(TestClass&& ob)
	{
		val = ob.val;
		test = ob.test;
//		ob.test = nullptr; lo necesito en destructor
		logLevel = ob.logLevel;		
		test->addTextToBuffer("TestClass move constructor\n",logLevel);
	}
	~TestClass()
	{
		test->addTextToBuffer("TestClass destructor\n",logLevel);
	}
	TestClass& operator=(const TestClass& ob)
	{
		val = ob.val;
		test = ob.test;
		logLevel = ob.logLevel;
		test->addTextToBuffer("TestClass copy operator=\n",logLevel);
		return *this;
	}
	TestClass& operator=(TestClass&& ob)
	{
		val = ob.val;
		test = ob.test;
//		ob.test = nullptr; lo necesito en destructor
		logLevel = ob.logLevel;
		test->addTextToBuffer("TestClass move operator=\n",logLevel);
		return *this;
	}
};
//funcion para pruebas a lo cerdo
int _testDebug(tests::BaseTest* test)
{
	int result = 0;	
	auto th1 = ThreadRunnable::create(true);	
	//auto th2 = ThreadRunnable::create(true);	
	execution::Executor<Runnable> exr(th1);		
	exr.setOpts({true,false,false});
	//now executor for threadpool
	parallelism::ThreadPool::ThreadPoolOpts opts;
	auto myPool = make_shared<parallelism::ThreadPool>(opts);
	parallelism::ThreadPool::ExecutionOpts exopts;
	execution::Executor<parallelism::ThreadPool> extp(myPool);
	extp.setOpts({true,true});   
	{
		TestClass p(test);
		p.val = 2;
		Future<TestClass&> fut(p);
		//fut.setValue(p);
		//fut.getValue().value().val = 10;
		text::info("{}",fut.getValue().value().val);
		text::info("{}",p.val);
		/*auto kk = execution::launch(ex,[](auto& v) -> vector<int>&
		{		
			v[1] = 4;
			return v; 
		},vec);*/
	}
	{
		vector<int> vec = {1,2,3};
		// auto kk0 = execution::launch(ex,[]()
		// {
		// });
		auto kk = execution::next(execution::inmediate(execution::start(exr),std::ref(vec)),[](auto& v) -> vector<int>&
		{		
			v.value()[1] = 4;
			return v.value();

		});
		//ESTO TAMBIEn
		// auto kk = execution::launch(ex,[](vector<int>& v) -> vector<int>&
		// {		
		// 	//v[1] = 4;
		// 	return v; 
		// },std::ref(vec));
		auto kk2 = execution::next(kk,[](auto& v)
		{
			tasking::Process::wait(1000);
			if ( v.isValid())
			{
				text::info("{}",v.value()[1]);
				v.value()[1] = 9;
				//text::info("{}",v.value());
			}else
			{
				text::error(v.error().errorMsg);
			}
		});
				
		core::waitForFutureThread(kk2);
		//Thread::sleep(200000);
		text::info("Val = {}",vec[1]);
		
	}

	
	
	
	// execution::launch(ex,
	// [](auto& v)
	// {
	// 	//vector<int> r = {5,6,7};
	// 	return 7;
	// },vec);
	//execution::next(execution::inmediate(execution::start(ex),std::move(vec)),
	string val = "dani";
	Future<string> fut;
	//tengo un problema, no sé cómo hacer que sea una referencia lo de dentro
//hay un problema muy gordo, el variatn no puede tener refen

	//fut.setValue(std::reference_wrapper(val));
	fut.setValue(val);
	auto fv = fut.getValue();
	text::info("Orig Value = {}",fut.getValue().value());
	text::info("Orig Value = {}",fv.value());
	fut.getValue().value() = "pepe"; 
	text::info("Value = {}",fut.getValue().value());
	text::info("Old Value = {}",val);

	
	{
		test->clearTextBuffer();
		TestClass pp(8,test);
		constexpr tests::BaseTest::LogLevel ll = tests::BaseTest::LogLevel::Info;
		pp.logLevel = ll;
		core::Event event;
		th1->fireAndForget([exr,&pp,&event,test,ll]{
			auto kk = execution::launch(exr,[]{
				return 7;
				//throw std::runtime_error("un error");
			});			
			auto kk0 = execution::inmediate(kk,std::ref(pp));
			auto kk1 = execution::next(kk0,[test,ll](auto& v)
			{
				if (v.isValid() )
				{					
					std::stringstream ss;
					ss << "Val = "<<v.value().val<<'\n';
					test->addTextToBuffer(ss.str(),ll); 
					//text::info("Val = {}",v.value().val);
					v.value().val = 10;
				}
				else
					text::info("Error = {}",v.error().errorMsg);					
			});			
			
			const auto& res = tasking::waitForFutureMThread(kk1);		
			if (res.isValid() )
			{
				//text::info("kk0 Val = {}",kk0.getValue().value().val);
				std::stringstream ss;
				ss << "kk0 Val = "<<kk0.getValue().value().val<<'\n';
				test->addTextToBuffer(ss.str(),ll); 
			}
			else
				text::info("Error = {}",kk0.getValue().error().errorMsg);				
			std::stringstream ss;
			ss << "Original Val = "<<pp.val<<'\n';
			test->addTextToBuffer(ss.str(),ll); 
			event.set();
		});
		event.wait();
		test->checkOccurrences("constructor",1,tests::BaseTest::LogLevel::Info);
		test->checkOccurrences("Val = 10",2,tests::BaseTest::LogLevel::Info);
		
	}
	{
	
		vector<float> vec = {1.0f,20.0f,36.5f};
		
		auto kk0 = execution::next(execution::inmediate(execution::start(exr),std::ref(vec)),[](auto& v) -> vector<float>&
		{
			auto& val = v.value();
			val[1] = 1000.7;
			//return std::ref(val);
			return val;
		}
		);
		//auto kk1 = execution::bulk(execution::inmediate(execution::start(exr),std::ref(vec)),[](auto& v)
		auto kk1 = execution::bulk(kk0,[](auto& v)
		{
			auto idx = v.index();
			//::tasking::Process::switchProcess(true);
			if ( v.isValid() )	
				text::info("Runnable Bulk 1 Value = {}",v.value()[0]);
			else
				text::info("Runnable Bulk 1 Error = {}",v.error().errorMsg);
			++v.value()[0];
		},[](auto& v)
		{
			if ( v.isValid() )	
				text::info("Runnable Bulk 2 Value = {}",v.value()[1]);
			else
				text::info("Runnable Bulk 2 Error = {}",v.error().errorMsg);
			++v.value()[1];
		},
		[](auto& v)
		{
			if ( v.isValid() )	
				text::info("Runnable Bulk 3 Value = {}",v.value()[2]);
			else
				text::info("Runnable Bulk 3 Error = {}",v.error().errorMsg);
			++v.value()[2];
		}
		);	
		core::waitForFutureThread(kk1);
		// auto kk1_1 = execution::next(kk1,[](auto& v)
		// {
		// 	text::info("After Bulk");			
		// 	if ( v.isValid() )
		// 	{
		// 		text::info("Value = {}",v.value()[1]);
		// 		v.value()[1] = 9.7f;
		// 		//text::info("After bulk value ({},{},{})",std::get<0>(val),std::get<1>(val),std::get<2>(val));
		// 	}else
		// 		text::info("After bulk err {}",v.error().errorMsg);
		// });
		// core::waitForFutureThread(kk1_1);
		text::info("After wait");
		// auto kk2 = execution::loop(kk1,idx0,loopSize,
		// 	[](int idx,const auto& v)
		// 	{
		// 		::tasking::Process::wait(1000);
		// 		if ( v.isValid() )
		// 		{
		// 			const auto& val = v.value();
		// 			text::info("It {}. Value = {}",idx,std::get<0>(val));				
		// 		}
		// 		else
		// 			text::info("It {}. Error = {}",idx,v.error().errorMsg);											
		// 	}
		// );
		// auto kk3 = execution::next(kk2,[](const auto& v)->int
		// {								
		// 	text::info("Launch waiting");
		// 	if ( ::tasking::Process::wait(5000) != tasking::Process::ESwitchResult::ESWITCH_KILL )
		// 	{
		// 		//throw std::runtime_error("Error1");
		// 		text::info("Launch done");
		// 	}else
		// 		text::info("Launch killed");
		// 	return 4;
		// }
		// );
		// ::core::waitForFutureThread(kk3);
		text::info("Done!!");
	}		
	{				   	
		vector<float> vec = {1.0f,20.0f,36.5f};
		//PROBAR ESTO Y LUEGO USAR EL kk0 EN EL BULK
		auto kk0 = execution::next(execution::inmediate(execution::start(extp),std::ref(vec)),[](auto& v) -> vector<float>&
		{
			v.value()[1] = 1000.7;
			return v.value();
		}
		);
		//auto kk1 = execution::bulk(execution::inmediate(execution::start(extp),std::ref(vec)),[](auto& v)
		auto kk1 = execution::bulk(kk0,[](auto& v)
		{
			auto idx = v.index();
			if ( v.isValid() )	
				text::info("ThreadPoolExecutor Bulk 1 Value = {}",v.value()[0]);
			else
				text::info("ThreadPoolExecutor Bulk 1 Error = {}",v.error().errorMsg);
			++v.value()[0];
		},[](auto& v)
		{
			if ( v.isValid() )	
				text::info("ThreadPoolExecutor Bulk 2 Value = {}",v.value()[1]);
			else
				text::info("ThreadPoolExecutor Bulk 2 Error = {}",v.error().errorMsg);
			++v.value()[1];
		},
		[](auto& v)
		{
			if ( v.isValid() )	
				text::info("ThreadPoolExecutor Bulk 3 Value = {}",v.value()[2]);
			else
				text::info("ThreadPoolExecutor Bulk 3 Error = {}",v.error().errorMsg);
			++v.value()[2];
		}
		);	
		core::waitForFutureThread(kk1);
		
	/*	auto kk0_1 = execution::next(kk0,[](auto& v)
		{
			text::info("After Bulk");			
			if ( v.isValid() )
			{
				text::info("Value = {}",v.value()[1]);
				v.value()[1] = 9.7f;
				//text::info("After bulk value ({},{},{})",std::get<0>(val),std::get<1>(val),std::get<2>(val));
			}else
				text::info("After bulk err {}",v.error().errorMsg);
		});
		core::waitForFutureThread(kk0_1);
		*/
		text::info("After wait");
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
		execution::Executor<Runnable> exr(th1);
		exr.setOpts({true,false,false});
		
		//---- basic checks
		{												
			core::Event event;
			//use a task to make it more complex
			th1->fireAndForget([exr,&event,test]
			{
				constexpr tests::BaseTest::LogLevel ll = tests::BaseTest::LogLevel::Info;
				test->clearTextBuffer();
				constexpr int initVal = 8;		
				const auto& res1 = tasking::waitForFutureMThread(
					execution::next(
						execution::inmediate(execution::start(exr),TestClass(initVal,test,ll))
					,[test,ll](auto& v)
					{
						if (v.isValid() )
						{					
							std::stringstream ss;
							ss << "Val = "<<v.value().val<<'\n';
							test->addTextToBuffer(ss.str(),ll); 
							//text::info("Val = {}",v.value().val);
							v.value().val = 10;
						}
						else
							text::info("Error = {}",v.error().errorMsg);					
					})
				);			
								
				if (res1.isValid() )
				{
					// std::stringstream ss;
					// ss << "kk0 Val = "<<kk0.getValue().value().val<<'\n';
					// test->addTextToBuffer(ss.str(),ll); 
				}
				else
					text::info("Error = {}",res1.error().errorMsg);				
					
				std::stringstream ss;
				ss << "Original Val = "<<initVal<<'\n';
				test->addTextToBuffer(ss.str(),ll); 
				test->checkOccurrences("TestClass constructor",1,tests::BaseTest::LogLevel::Info);

				
				test->checkOccurrences("TestClass copy",0,tests::BaseTest::LogLevel::Info);
				
				//now using reference
				test->clearTextBuffer();
				TestClass pp(8,test,ll);
				pp.logLevel = ll;
				auto kk = execution::launch(exr,[]{
					return 7;
					//throw std::r//tasking::waitForFutureMThread(tt);untime_error("un error");
				});			
				auto kk0 = execution::inmediate(kk,std::ref(pp));
				auto kk1 = execution::next(kk0,[test,ll](auto& v)
				{
					if (v.isValid() )
					{					
						std::stringstream ss;
						ss << "Val = "<<v.value().val<<'\n';
						test->addTextToBuffer(ss.str(),ll); 
						//text::info("Val = {}",v.value().val);
						v.value().val = 10;
					}
					else
						text::info("Error = {}",v.error().errorMsg);					
				});			
				
				const auto& res2 = tasking::waitForFutureMThread(kk1);		
				if (res2.isValid() )
				{
					//text::info("kk0 Val = {}",kk0.getValue().value().val);
					std::stringstream ss;
					ss << "kk0 Val = "<<kk0.getValue().value().val<<'\n';
					test->addTextToBuffer(ss.str(),ll); 
				}
				else
					text::info("Error = {}",kk0.getValue().error().errorMsg);				
				ss.str(""s);
				ss << "Original Val = "<<pp.val<<'\n';
				test->addTextToBuffer(ss.str(),ll); 
				event.set();
				test->checkOccurrences("constructor",1,tests::BaseTest::LogLevel::Info);
				test->checkOccurrences("Val = 10",2,tests::BaseTest::LogLevel::Info);
			});
			event.wait();
			
		}
		{
		const int idx0 = 0;
		const int loopSize = 10;
	/*	auto kk1 = execution::launch(ex,
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
		*/
		}
		//---
		// auto fut = execution::next(execution::start(ex),
		// 			[](const auto& v)->int
		// 			{					
		// 				text::info("Current Runnable {}",static_cast<void*>(ThreadRunnable::getCurrentRunnable()));
		// 				text::info("Launch waiting");
		// 				if ( ::tasking::Process::wait(5000) != tasking::Process::ESwitchResult::ESWITCH_KILL )
		// 				{
		// 					//throw std::runtime_error("Error1");
		// 					text::info("Launch done");
		// 				}else
		// 					text::info("Launch killed");
		// 				return 4;
		// 			}
		// 		);
		// //Thread::sleep(10000);
		// execution::Executor<Runnable> extp(th2);
		// //::core::waitForFutureThread(fut);	
		// //auto fut_1 = execution::transfer(fut,extp);
		// fut = execution::transfer(fut,extp);
		// auto fut2 = execution::next(fut,[](const auto& v)
		// 		{
		// 			text::info("Current Runnable {}",static_cast<void*>(ThreadRunnable::getCurrentRunnable()));
		// 			if (v.isValid())
		// 			{
		// 				text::info("Next done: {}",v.value());		
		// 			}else					
		// 				text::error("Next error: {}",v.error().errorMsg);		
		// 		}
		// 		);						
		// ::core::waitForFutureThread(fut2);				
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
		/*auto kk1 = execution::launch(ex,
			[]()
			{		
				text::info("Launch TP kk");			
				::tasking::Process::wait(5000);					
				//throw std::runtime_error("Error1");
				return "pepe";
			}
		);*/
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
		//auto fut = execution::launch(ex,
		// 	[](string v)
		// 	{					
		// 		text::info("Tp Launch waiting");
		// 		if ( ::tasking::Process::wait(5000) != tasking::Process::ESwitchResult::ESWITCH_KILL )
		// 			text::info("TP Launch done: {}",v);
		// 		else
		// 			text::info("TP Launch killed");
		// 		return 4;
		// 	},"pepe"
		// );
		// text::info("TP Sleep");
		// Thread::sleep(10000);
		// auto fut2 = execution::next(fut,[](const auto& v)->void
		// 		{
		// 			if (v.isValid())
		// 			{
		// 				text::info("TP Next done: {}",v.value());		
		// 			}else					
		// 				text::error("TP Next error: {}",v.error().errorMsg);		
		// 		}
		// 		);		
		// ::core::waitForFutureThread(fut2);                

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
			execution::Executor<parallelism::ThreadPool> extp(myPool);
			auto r2 = extp.launch<int>(  
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
	// 		execution::Executor<parallelism::ThreadPool> extp(myPool);
	// 		execution::ThreadPoolExecutorOpts exopts;
	// 		exopts.independentTasks = false;
	// 		extp.setOpts(exopts);
	// 		const int idx0 = 0;
	// 		auto barrier = execution::loop(extp,idx0,loopSize,gTestFunc);
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
			execution::Executor<parallelism::ThreadPool> extp(myPool);
			execution::LoopHints lhints;
			lhints.independentTasks = true;
			const int idx0 = 0;
			auto barrier = extp.loop(idx0,loopSize,
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
					_testDebug(this);
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