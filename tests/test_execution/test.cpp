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
#include <vector>
using std::vector;

const std::string TestExecution::TEST_NAME = "execution";

//test for development purposes
namespace test_execution
{
struct MyErrorInfo : public ::core::ErrorInfo
{
	static tests::BaseTest* test;
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
tests::BaseTest* MyErrorInfo::test = nullptr;
}
/*
 * class for testing copies, moves, etc..
 * 
 */
struct TestClass
{
	float val;
	tests::BaseTest* test;
	tests::BaseTest::LogLevel logLevel;
	bool addToBuffer;
	TestClass(tests::BaseTest* aTest,tests::BaseTest::LogLevel ll = tests::BaseTest::LogLevel::Info,bool atb = true):val(1),test(aTest),logLevel(ll),addToBuffer(atb)
	{
		_printTxt("TestClass constructor\n");
	}
	explicit TestClass(float aVal,tests::BaseTest* aTest,tests::BaseTest::LogLevel ll = tests::BaseTest::LogLevel::Info,bool atb = true):val(aVal),test(aTest),logLevel(ll),addToBuffer(atb)
	{
		_printTxt("TestClass constructor\n");
	}
	TestClass(const TestClass& ob)
	{
		val = ob.val;
		test = ob.test;
		logLevel = ob.logLevel;
		addToBuffer = ob.addToBuffer;
		_printTxt("TestClass copy constructor\n");
	}
	TestClass(TestClass&& ob)
	{
		val = ob.val;
		ob.val = -1;
		test = ob.test;
//		ob.test = nullptr; lo necesito en destructor
		logLevel = ob.logLevel;		
		addToBuffer = ob.addToBuffer;
		_printTxt("TestClass move constructor\n");
	}
	~TestClass()
	{
		_printTxt("TestClass destructor\n");
	}
	TestClass& operator=(const TestClass& ob)
	{
		val = ob.val;
		test = ob.test;
		logLevel = ob.logLevel;
		addToBuffer = ob.addToBuffer;
		_printTxt("TestClass copy operator=\n");
		return *this;
	}
	TestClass& operator=(TestClass&& ob)
	{
		val = ob.val;
		ob.val = -1;
		test = ob.test;
//		ob.test = nullptr; lo necesito en destructor
		logLevel = ob.logLevel;
		addToBuffer = ob.addToBuffer;
		_printTxt("TestClass move operator=\n");
		return *this;
	}
	private:
	void _printTxt(const string& str)
	{
		if (addToBuffer && test)
			test->addTextToBuffer(str,logLevel);
		else
		{
			switch (logLevel)
			{
				case tests::BaseTest::LogLevel::Debug:
					text::debug(str);
					break;
				case tests::BaseTest::LogLevel::Info:
					text::info(str);
					break;
				case tests::BaseTest::LogLevel::Warn:
					text::warn(str);
					break;
				case tests::BaseTest::LogLevel::Error:
					text::error(str);
					break;
				case tests::BaseTest::LogLevel::Critical:
					text::critical(str);
					break;
				default:break;
			}
		}
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
		int var = 7;
		
		auto f = 
		execution::launch<test_execution::MyErrorInfo>(exr,
		[](int& arg)->int&
		{
			//throw std::runtime_error("ERR EN LAUNCH");
			arg=9;
			return arg;
		},std::ref(var))	
		//execution::start(exr)
		| execution::next([](int& arg)
		{
			arg = 20;
			throw std::runtime_error("ERR EN NEXT1");

			return arg + 5;
		}) 
		
		| execution::next([](int& v)
		{
	//		throw std::runtime_error("ERR EN NEXT2");
			return v+5;
		})
		| execution::getError([](const auto& err)
		{
			text::info("Hay error {}", err.errorMsg);
			return 8;
		})
		| execution::bulk(
			[](int v)
			{
				text::info("BULK 1 {}",v);
				//return v.value()+5;
			},
			[](int v)
			{
				text::info("BULK 2 {}",v);
				//return v.value()+5;
			}
		)
		| execution::loop(0,10,[](int idx,int& v)
		{
			text::info("IT {}. Value = {}",idx,v);
		})
		| execution::next([](int& v)
		{
			throw std::runtime_error("ERR EN NEXT3");
			text::info("V = {}",v);
		})
		| execution::loop(0,10,[](int& idx)
		{
			text::info("IT2 {}. ");
		}) 
		| execution::getError([](const auto& err)
		{
			text::info("HAY ERROR AL FINAL. {}",err.errorMsg);
		})
		;
		auto res = ::core::waitForFutureThread(f);
		/*if ( res.isValid())
		{
			text::info("RES = {}",res.value());
		}else
			text::info("ERR = {}",res.error().errorMsg);
			*/
		if ( !res.isValid())  //void result		
			text::info("ERR = {}",res.error().errorMsg);
		text::info("FIN");
	}
	
	
	
	
	// {
	
	// 	vector<float> vec = {1.0f,20.0f,36.5f};
		
	// 	auto kk0 = execution::next(execution::inmediate(execution::start(exr),std::ref(vec)),[](auto& v) -> vector<float>&
	// 	{
	// 		auto& val = v.value();
	// 		val[1] = 1000.7;
	// 		//return std::ref(val);
	// 		return val;
	// 	}
	// 	);
	// 	//auto kk1 = execution::bulk(execution::inmediate(execution::start(exr),std::ref(vec)),[](auto& v)
	// 	auto kk1 = execution::bulk(kk0,[](auto& v)
	// 	{
	// 		auto idx = v.index();
	// 		//::tasking::Process::switchProcess(true);
	// 		if ( v.isValid() )	
	// 			text::info("Runnable Bulk 1 Value = {}",v.value()[0]);
	// 		else
	// 			text::info("Runnable Bulk 1 Error = {}",v.error().errorMsg);
	// 		++v.value()[0];
	// 	},[](auto& v)
	// 	{
	// 		if ( v.isValid() )	
	// 			text::info("Runnable Bulk 2 Value = {}",v.value()[1]);
	// 		else
	// 			text::info("Runnable Bulk 2 Error = {}",v.error().errorMsg);
	// 		++v.value()[1];
	// 	},
	// 	[](auto& v)
	// 	{
	// 		if ( v.isValid() )	
	// 			text::info("Runnable Bulk 3 Value = {}",v.value()[2]);
	// 		else
	// 			text::info("Runnable Bulk 3 Error = {}",v.error().errorMsg);
	// 		++v.value()[2];
	// 	}
	// 	);	
	// 	core::waitForFutureThread(kk1);
	// 	// auto kk1_1 = execution::next(kk1,[](auto& v)
	// 	// {
	// 	// 	text::info("After Bulk");			
	// 	// 	if ( v.isValid() )
	// 	// 	{
	// 	// 		text::info("Value = {}",v.value()[1]);
	// 	// 		v.value()[1] = 9.7f;
	// 	// 		//text::info("After bulk value ({},{},{})",std::get<0>(val),std::get<1>(val),std::get<2>(val));
	// 	// 	}else
	// 	// 		text::info("After bulk err {}",v.error().errorMsg);
	// 	// });
	// 	// core::waitForFutureThread(kk1_1);
	// 	text::info("After wait");
	// 	// auto kk2 = execution::loop(kk1,idx0,loopSize,
	// 	// 	[](int idx,const auto& v)
	// 	// 	{
	// 	// 		::tasking::Process::wait(1000);
	// 	// 		if ( v.isValid() )
	// 	// 		{
	// 	// 			const auto& val = v.value();
	// 	// 			text::info("It {}. Value = {}",idx,std::get<0>(val));				
	// 	// 		}
	// 	// 		else
	// 	// 			text::info("It {}. Error = {}",idx,v.error().errorMsg);											
	// 	// 	}
	// 	// );
	// 	// auto kk3 = execution::next(kk2,[](const auto& v)->int
	// 	// {								
	// 	// 	text::info("Launch waiting");
	// 	// 	if ( ::tasking::Process::wait(5000) != tasking::Process::ESwitchResult::ESWITCH_KILL )
	// 	// 	{
	// 	// 		//throw std::runtime_error("Error1");
	// 	// 		text::info("Launch done");
	// 	// 	}else
	// 	// 		text::info("Launch killed");
	// 	// 	return 4;
	// 	// }
	// 	// );
	// 	// ::core::waitForFutureThread(kk3);
	// 	text::info("Done!!");
	// }		
	

	Thread::sleep(5000);
	return 0;
}

template <class ExecutorType> void _basicTests(ExecutorType ex,ThreadRunnable* th,tests::BaseTest* test)
{
	core::Event event;
	TestClass pp(8,test);
	vector<TestClass> vec;
	//use a task to make it more complex
	th->fireAndForget([ex,&event,test,&pp,&vec] () mutable
	{
		constexpr tests::BaseTest::LogLevel ll = tests::BaseTest::LogLevel::Debug;
		text::info("Simple functor chaining. using operator | from now");
		test->clearTextBuffer();
		constexpr int initVal = 8;		
		{
			
		auto th2 = ThreadRunnable::create();
		execution::Executor<Runnable> ex2(th2);		
		auto res1 = tasking::waitForFutureMThread(
			execution::start(ex)
			| execution::inmediate(TestClass(initVal,test,ll)) 				
			| execution::bulk(
				[](TestClass& v)
				{
					text::debug("Bulk 1");					
					auto prevVal = v.val;
					v.val = 1;  //race condition
					tasking::Process::wait(1000);
					v.val = prevVal + 5;
				},
				[](TestClass& v)
				{
					text::debug("Bulk 2");
					tasking::Process::wait(300);
					v.val = 12; //Bulk 1 should be the last to execute its assignment 
				})  
		//	| execution::transfer(ex2)  //transfer execution to a different executor
			| execution::next([test,ll](TestClass& v)
			{
				//text::info("Val = {}",v.value().val);
				v.val+= 3;
				return std::move(v); //avoid copy constructor			
			})
		);		
		
		bool iv = res1.isValid();
		if (res1.isValid() )
		{
			int finalVal = res1.value().val;
			int expectedVal = initVal + 5+3;
			text::debug("Finish Val = {}",finalVal);
			if (finalVal != expectedVal)
			{
				std::stringstream ss;
				ss << " final value after chain of execution is no correct. Expected "<<expectedVal << " got "<<finalVal;
				test->setFailed(ss.str());
			}
		}
		else
			text::info("Error = {}",res1.error().errorMsg);				
		}		
		test->checkOccurrences("TestClass constructor",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);		
		test->checkOccurrences("TestClass copy",0,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);							
		test->checkOccurrences("destructor",test->findTextInBuffer("constructor"),__FILE__,__LINE__);
			
		// 	base form to do the same
		// 	execution::next(
		// 	execution::transfer(
		// 	execution::bulk(
		// 		execution::inmediate(execution::start(ex),TestClass(initVal,test,ll))
		// 	,
		// 	//bulk
		// 	[](auto& v)
		// 	{
		// 		text::info("Bulk 1");
		// 		v.value().val = 11;  //race condition
		// 	},
		// 	[](auto& v)
		// 	{
		// 		text::info("Bulk 2");
		// 		v.value().val = 12; //race condition
		// 	}),
		// 	//transfer
		// 	ex2), 
		// 	//next
		// 	[test,ll](auto& v) -> TestClass&
		// 	{
		// 		if (v.isValid() )
		// 		{					
		// 			std::stringstream ss;
		// 			ss << "Val = "<<v.value().val<<'\n';
		// 			test->addTextToBuffer(ss.str(),ll); 
		// 			//text::info("Val = {}",v.value().val);
		// 			v.value().val = 10;
		// 		}
		// 		else
		// 			text::info("Error = {}",v.error().errorMsg);					
		// 		return v.value();
		// 	})
		// );	
					
/*
		//now using reference
		test->clearTextBuffer();
		pp.logLevel = ll;
		text::info("Simple functor chaining using reference");
	
		auto res2 = tasking::waitForFutureMThread(
			execution::launch(ex,[](TestClass& tc) -> TestClass&
			{
				tc.val = 7;
				return tc;
				//throw std::runtime_error("chiquilin");
			},std::ref(pp)) 
			| execution::next([test,ll](auto& v) -> TestClass&
			{
				if (v.isValid() )
				{					
					v.value().val += 10;
					return v.value();
				}
				else
				{
					throw std::runtime_error( v.error().errorMsg );
				}
			}));
		if (res2.isValid() )
		{
			int finalVal = res2.value().val;
			int expectedVal = 17;
			text::debug("Finish Val = {}",finalVal);
			if (finalVal != expectedVal)
			{
				std::stringstream ss;
				ss << " final value after chain of execution is no correct. Expected "<<expectedVal << " got "<<finalVal;
				test->setFailed(ss.str());
			}
			if ( finalVal != pp.val)
			{
				std::stringstream ss;
				ss << " final value is diferent than original value. ";
				test->setFailed(ss.str());
			}
		}
		else
			text::info("Error = {}",res2.error().errorMsg);				
		
		test->checkOccurrences("constructor",0,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		test->checkOccurrences("destructor",test->findTextInBuffer("constructor"),__FILE__,__LINE__);
		//now a little more complex test //TODO: meter al final un throw o algo así
		text::info("A little more complex functor chaining using reference and parallel loops");
		test->clearTextBuffer();
		
		#define LOOP_SIZE 100
		#define INITIAL_VALUE 2
		auto res3 = tasking::waitForFutureMThread(
			execution::start(ex) 
			| execution::inmediate(std::ref(vec)) 
			| execution::next([test,ll](auto& v)->vector<TestClass>&
				{				
					//fill de vector with a simple case for the result to be predecible
					//I don't want out to log the initial constructions, oncly constructons and after this function
					auto t = TestClass(INITIAL_VALUE,test,tests::BaseTest::LogLevel::None,false);
					v.value().resize(LOOP_SIZE,t);	
					for(auto& elem:v.value())
					{
						elem.logLevel = ll;
						elem.addToBuffer = true;
					}
					return v.value();
				}
				)
				| execution::next([](auto& v)->vector<TestClass>&
				{
					if (v.isValid() )
					{					
						for(auto& elem:v.value())
							++elem.val;						
					}
					else
						text::info("Error = {}",v.error().errorMsg);	
					return v.value();	
				})
				| execution::bulk([](auto& v)
				{					
					//multiply by 2 the first half
					if (v.isValid() )
					{					
						auto& vec = v.value();
						size_t endIdx = vec.size()/2;	
						for(size_t i = 0; i < endIdx;++i )
						{
							vec[i].val = vec[i].val*2.f;	
						}
					}
					else
						text::info("Bulk Error = {}",v.error().errorMsg);					
				},
				[](auto& v)			
				{
					//multiply by 3 the second half
					if (v.isValid() )
					{					
						auto& vec = v.value();
						size_t startIdx = vec.size()/2;
						for(size_t i = startIdx; i < vec.size();++i )
						{
							vec[i].val = vec[i].val*3.f;
						}
					}
					else
						text::info("Bulk Error = {}",v.error().errorMsg);										
				}) 
				| execution::loop(
					0,LOOP_SIZE,
					[](int idx,auto& v)
					{
						if ( v.isValid())
						{
							v.value()[idx].val+=5.f;
						}
					},1
				)
				| execution::next(
					[](auto& v)->vector<TestClass>&
					{					
						if (v.isValid() )
						{					
							for(auto& elem:v.value())
								++elem.val;		
						}	
						return v.value();				
					}
		));
			
		test->checkOccurrences("constructor",0,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		test->checkOccurrences("copy",0,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		stringstream ss;
		ss<<"Valor vector: ";
		for(const auto& v:vec)
			ss << v.val<<' ';
		ss << '\n';
		test->addTextToBuffer(ss.str(),tests::BaseTest::LogLevel::None); 
		if (!res3.isValid() )
			text::info("Error = {}",res3.error().errorMsg);
		else
		{
			//compare result with original vector. Must be the same
			const auto& v = res3.value(); 
			if ( &v != &vec )
				test->setFailed("Both vectors must be the same ");
		}
	
		test->checkOccurrences(std::to_string((INITIAL_VALUE+1)*2+5+1),vec.size()/2,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		test->checkOccurrences(std::to_string((INITIAL_VALUE+1)*3+5+1),vec.size()/2,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		
		text::info("Same process as the previous without using reference");
		//ss.str(""s); //empty stream
		test->clearTextBuffer();
		auto res4 = tasking::waitForFutureMThread(
			execution::start(ex) 
			| execution::next([test,ll](const auto& v)
			{				
				return vector<TestClass>();
			})
				| execution::next([test,ll](auto& v)
				{				
					//fill de vector with a simple case for the result to be predecible
					//I don't want out to log the initial constructions, oncly constructons and after this function
					auto t = TestClass(INITIAL_VALUE,test,tests::BaseTest::LogLevel::None,false);
					v.value().resize(LOOP_SIZE,t);	
					for(auto& elem:v.value())
					{
						elem.logLevel = ll;
						elem.addToBuffer = true;
					}
					return std::move(v.value()); 
				}
				 )
				| execution::next([](auto& v)
				{
					if (v.isValid() )
					{	
						size_t s = v.value().size();
						for(auto& elem:v.value())
							++elem.val;						
					}
					else
						text::info("Error = {}",v.error().errorMsg);	
					return std::move(v.value());	
				})
				| execution::bulk([](auto& v)
				{					
					//multiply by 2 the first half
					if (v.isValid() )
					{					
						auto& vec = v.value();
						size_t endIdx = vec.size()/2;	
						for(size_t i = 0; i < endIdx;++i )
						{
							vec[i].val = vec[i].val*2.f;	
						}
					}
					else
						text::info("Bulk Error = {}",v.error().errorMsg);					
				},
				[](auto& v)			
				{
					//multiply by 3 the second half
					if (v.isValid() )
					{					
						auto& vec = v.value();
						size_t startIdx = vec.size()/2;
						for(size_t i = startIdx; i < vec.size();++i )
						{
							vec[i].val = vec[i].val*3.f;
						}
					}
					else
						text::info("Bulk Error = {}",v.error().errorMsg);										
				}) 
				| execution::loop(
					0,LOOP_SIZE,
					[](int idx,auto& v)
					{
						if ( v.isValid())
						{
							auto& vec = v.value();
							vec[idx].val+=5.f;
						}
					},1
				)
				| execution::next(
					[](auto& v)
					{					
						if (v.isValid() )
						{					
							for(auto& elem:v.value())
								++elem.val;		
						}	
						return std::move(v.value());				
					})
		);
		
		test->checkOccurrences("constructor",0,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		test->checkOccurrences("copy",0,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		
		if (!res4.isValid() )
			text::info("Error = {}",res4.error().errorMsg);
		else
		{
			stringstream ss;
			ss<<"Valor vector: ";
			for(const auto& v:res4.value())
				ss << v.val<<' ';
			ss << '\n';
			test->addTextToBuffer(ss.str(),tests::BaseTest::LogLevel::None); 
			//compare result with original vector. Must be the same
			const auto& v = res4.value(); 
			if ( &v == &vec )
				test->setFailed("Both vectors NUST NOT be the same ");
			test->checkOccurrences(std::to_string((INITIAL_VALUE+1)*2+5+1),res4.value().size()/2,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
			test->checkOccurrences(std::to_string((INITIAL_VALUE+1)*3+5+1),res4.value().size()/2,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		}
		*/	
		event.set();
	});
	event.wait();
}

//basic test for launching task in execution agents
int _testLaunch( tests::BaseTest* test)
{
	int result = 0;		
	{
		{
			auto th1 = ThreadRunnable::create(true);			
			execution::Executor<Runnable> exr(th1);
			exr.setOpts({true,false,false});
			text::info("\n\tBasicTests with RunnableExecutor");
			_basicTests(exr,th1.get(),test);
			text::info("\n\tFinished BasicTests with RunnableExecutor");
		}
		{
			auto th1 = ThreadRunnable::create(true);						
			parallelism::ThreadPool::ThreadPoolOpts opts;
			auto myPool = make_shared<parallelism::ThreadPool>(opts);
			parallelism::ThreadPool::ExecutionOpts exopts;
			execution::Executor<parallelism::ThreadPool> extp(myPool);
			extp.setOpts({true,true});
			text::info("\n\tBasicTests with ThreadPoolExecutor");
		//	_basicTests(extp,th1.get(),test);
			text::info("\n\tFinished BasicTests with ThreadPoolExecutor");
		}
	}
		
	/*
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
	}*/
	/*
	{		
		parallelism::ThreadPool::ThreadPoolOpts opts;
		auto myPool = make_shared<parallelism::ThreadPool>(opts);
		parallelism::ThreadPool::ExecutionOpts exopts;
		execution::Executor<parallelism::ThreadPool> ex(myPool);
		ex.setOpts({true,true});
		{
		const int idx0 = 0;
		const int loopSize = 10;		
		// auto kk1 = execution::launch(ex,
		// 	[]()
		// 	{		
		// 		text::info("Launch TP kk");			
		// 		::tasking::Process::wait(5000);					
		// 		//throw std::runtime_error("Error1");
		// 		return "pepe";
		// 	}
		// );
		// auto kk = execution::loop(kk1,idx0,loopSize,
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
auto gTestFunc=[](int idx,const auto&)
	{
		text::debug("iteracion pre {}",idx);
		::tasking::Process::switchProcess(true);
		text::debug("iteracion post {}",idx);
	};
// loop test to measure performance
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
	// 		core::waitForFutureThread(execution::loop(execution::start(ex),idx0,loopSize,gTestFunc,1));
	// 		//text::debug("hecho");
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
	// 		execution::RunnableExecutorOpts exopts;
	// 		exopts.independentTasks = true;
	// 		exopts.lockOnce = false;
	// 		ex.setOpts(exopts);
	// 		const int idx0 = 0;
	// 		// auto barrier = ex.loop(idx0,loopSize,gTestFunc,lhints,1
	// 		// );
	// 		core::waitForFutureThread(execution::loop(execution::start(ex),idx0,loopSize,gTestFunc,1));
			
	// 	},loopSize
	// );

	// _measureTest("Runnable executor with independent tasks,lockOnce and pausing thread",
	// 	[loopSize]() mutable
	// 	{
	// 		auto th1 = ThreadRunnable::create(false,CHUNK_SIZE);
	// 		execution::Executor<Runnable> ex(th1);	
	// 		execution::RunnableExecutorOpts exopts;
	// 		exopts.independentTasks = true;
	// 		exopts.lockOnce = true;
	// 		int idx0 = 0;
	// 		auto r = execution::loop(execution::start(ex),idx0,loopSize,gTestFunc,1);
	// 		th1->resume();
	// 		core::waitForFutureThread(r);
	// 		th1->suspend();
	// 	},loopSize
	// );
	// _measureTest("Runnable executor with independent tasks,NO lockOnce on post and pausing thread",
	// 	[loopSize]() 	
	// 	{
	// 		auto th1 = ThreadRunnable::create(false,CHUNK_SIZE);
	// 		execution::Executor<Runnable> ex(th1);	
	// 		execution::RunnableExecutorOpts exopts;
	// 		exopts.independentTasks = true;
	// 		exopts.lockOnce = false;
	// 		const int idx0 = 0;
	// 		auto r = execution::loop(execution::start(ex),idx0,loopSize,gTestFunc,1);
	// 		th1->resume();
	// 		core::waitForFutureThread(r);
	// 		th1->suspend();
	// 	},loopSize
	// );
	// _measureTest("Runnable executor WITHOUT independent tasks",
	// 	[loopSize]()
	// 	{
	// 		auto th1 = ThreadRunnable::create(true,CHUNK_SIZE);
	// 		e´xopts.lockOnce = true;
	// 		const int idx0 = 0;
	// 		core::waitForFutureThread(execution::loop(execution::start(ex),idx0,loopSize,gTestFunc,1));
	// 	},loopSize
	// );
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
	// 		core::waitForFutureThread(execution::loop(execution::start(extp),idx0,loopSize,gTestFunc,1));
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