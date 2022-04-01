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

/*
example code to detect MSVC compile error in some version related with  /permissive- and /Zc:lambda flags
struct MyClass
{
    //no working in MSVC
    template <class TArg,class F> void test(F&& f,TArg&& arg) noexcept( sizeof(TArg)>0)
    {
        //This compile in MSVC
        if constexpr (sizeof(TArg)>1)
        {
            
        }
        //But this next doesn't compile in MSVC
        // auto g = []() noexcept( std::is_integral<TArg>::value)
        // {
        // };
        auto g = []() noexcept( sizeof(TArg) > 1)
        {
        };
        
    }
};

int f1(float&)
{
    return 6;
}
void f()
{
 MyClass mc1;
    float var = 7.1f;
    mc1.test(f1,var);

}
*/
//test for development purposes
namespace test_execution
{
struct MyErrorInfo 
{
	static tests::BaseTest* test;
	MyErrorInfo(int code,string msg):error(code),errorMsg(std::move(msg))
	{
		text::debug("MyErrorInfo");
	}
	MyErrorInfo(MyErrorInfo&& ei):error(ei.error),errorMsg(std::move(ei.errorMsg)){}
	MyErrorInfo(const MyErrorInfo& ei):error(ei.error),errorMsg(ei.errorMsg){}
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
	int error;
	string errorMsg;
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
	TestClass():test(nullptr){}
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
		{	

			auto idc = std::is_default_constructible<TestClass>::value;
			auto f1 = 
			execution::launch(extp,
			[test](int arg) noexcept
			{
				//throw test_execution::MyErrorInfo(0,"usando MyErrorInfo");
				return TestClass (8,test);
			},7) |
			execution::next([](TestClass& tc) 
			{
				//throw std::runtime_error("ERR EN NEXT");
				//throw test_execution::MyErrorInfo(0,"usando MyErrorInfo");
				text::info("Next");
				return tc;
			}
			) 
			|  execution::parallel(
				[](TestClass& tc) noexcept 
				{
					text::info("B1");					
					//throw std::runtime_error("ERR EN parallel");
					//throw test_execution::MyErrorInfo(0,"usando MyErrorInfo");
				}
				,[](TestClass& tc)  
				{					
					//throw std::runtime_error("ERR EN parallel 2");
					text::info("B2");
				}
				)
			| execution::parallel_convert<std::tuple<int,float>>(
				[](TestClass& tc) noexcept
				{
					::tasking::Process::wait(100);
					//throw std::runtime_error("ERR EN parallel");
					return 1;
				},[](TestClass& tc)
				{
					//throw test_execution::MyErrorInfo(0,"usando MyErrorInfo");
					return 6.7f;
				}
			)
			| execution::catchError([](std::exception_ptr err) 
			{
				//throw std::runtime_error("ERR EN catchError");
				std::rethrow_exception(err);
				return std::tuple<int,float>(1,1.1f);
			})
			// | execution::catchError([](std::exception_ptr err) noexcept
			// {
			// 	return std::tuple<int,float>(2,2.1f);
			// })
			| execution::loop(0,10,[](int idx,std::tuple<int,float>& input) noexcept
			{
				text::info("Loop it {}. ",idx);
				// if ( idx == 4 )
				// 	throw test_execution::MyErrorInfo(idx,"usando MyErrorInfo");
				std::get<1>(input)=10.4f;
			})
			| execution::next([](std::tuple<int,float>& input)
			{
				text::info("Values: [{}.{}] ",std::get<0>(input),std::get<1>(input));
				return TestClass(11,nullptr);
			})
			;
			// try
			// {
			// 	auto res = core::waitForFutureThread(f1);
			// 	text::info("Wait ok...");
			// }catch(std::exception& e)
			// {
			// 	text::error("Error waiting for result.Reason = {}",e.what());
			// }
			// catch(test_execution::MyErrorInfo& e)
			// {
			// 	text::error("Error (MyErrorInfo)waiting for result.Reason = {}",e.errorMsg);
			// }
			// catch(...)
			// {
			// 	text::error("Error waiting for result.Reason = unknown");
			// }
			auto f2 = 
			execution::launch(exr,
			[]()
			{
				//throw std::runtime_error("ERR EN LAUNCH");
				throw test_execution::MyErrorInfo(0,"usando MyErrorInfo");
				return "pepe";
			});
			
			auto f3 = execution::on_all(exr,f1,f2);
			try
			{
				auto res = core::waitForFutureThread(f3);
				const auto& val = res.value();
				text::info("tras on_all[ {}, {} ]",std::get<0>(val).val,std::get<1>(val));				
			}
			catch(execution::OnAllException& e)
			{
				text::error("OnAllException: Error {} in element {}.", e.what(),e.getWrongElement());
				try
				{
					std::rethrow_exception(e.getCause());
				}catch(std::exception& e)
				{
					text::error("	Cause = {}",e.what());
				}
				catch(test_execution::MyErrorInfo& e)
				{
					text::error("	Cause = {}",e.errorMsg);
				}
				catch(...)
				{
					text::error("	Cause = Unknown");
				}
			}
			catch(std::exception& e)
			{
				text::info("Error {}", e.what());
			}
			catch(test_execution::MyErrorInfo& e)
			{
				text::error("Error (MyErrorInfo)waiting for result.Reason = {}",e.errorMsg);
			}
			catch(...)
			{
				text::error("Error waiting for result.Reason = unknown");
			}
		}
		int var = 7;
		auto f_0 = 
		execution::launch(exr,
		[](int& arg) noexcept ->int&
		{
			//throw std::runtime_error("ERR EN LAUNCH");
			//throw test_execution::MyErrorInfo(0,"ERR EN LAUNCH");
			arg=9;
			return arg;
		},std::ref(var))	
		| execution::catchError( [&var](std::exception_ptr err)-> int&
		{
			return var;
		})
		
		;
		/*auto f = f_0 | execution::transfer(extp) | execution::parallel_convert<std::tuple<TestClass,string>>(
			[test](int n)
			{
				//text::info("BULK 1 {}",v);
				//return v.value()+5;
				return TestClass(7,test);
			},
			[](int n)
			{
				//text::info("BULK 2 {}",v);
				//return v.value()+5;
				return "pepe";
			}
		);*/

		try
		{
			auto res = ::core::waitForFutureThread(f_0);
			//text::info("Value = [{},{}]",std::get<0>(res.value()).val,std::get<1>(res.value()));
		}catch(std::exception& e)
		{
			text::info("ERR = {}",e.what());
		}

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
	// 	//auto kk1 = execution::parallel(execution::inmediate(execution::start(exr),std::ref(vec)),[](auto& v)
	// 	auto kk1 = execution::parallel(kk0,[](auto& v)
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
	// 	// 		//text::info("After parallel value ({},{},{})",std::get<0>(val),std::get<1>(val),std::get<2>(val));
	// 	// 	}else
	// 	// 		text::info("After parallel err {}",v.error().errorMsg);
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
	// 	// 		text::info("LauncstartIdx
	// 	// ::core::waitForFutureThread(kk3);
	// 	text::info("Done!!");
	// }		
	

	Thread::sleep(5000);
	return 0;
}
//code for the samples in the documentation
template <class ExecutorType> void _sampleBasic(ExecutorType ex)
{	
	auto th = ThreadRunnable::create();
	th->fireAndForget([ex] () mutable
	{
		auto res = ::tasking::waitForFutureMThread(
			execution::launch(ex,[](float param) noexcept
			{	
				return param+6.f;
			},10.5f)
			| execution::next( [](float& param) noexcept
			{
				return std::to_string(param);
			})
			| execution::parallel( 
				[](string& str)
				{					
					text::info("Parallel 1. {}",str+" hello!");
				},
				[](string& str)
				{
					text::info("Parallel 2. {}",str+" hi!");
				},
				[](string& str)
				{
					text::info("Parallel 2. {}",str+" whats up!");
				}
			)
		);
		if (res.isValid())
		{
			::text::info("Result value = {}",res.value());
		}
	},0,::tasking::Runnable::_killFalse);
}
template <class ExecutorType> void _sampleReference(ExecutorType ex)
{	
	string str = "Hello";
	auto th = ThreadRunnable::create();
	/*th->fireAndForget([ex,&str] () mutable
	{
		auto res = ::tasking::waitForFutureMThread(
			execution::launch(ex,[](string& str) noexcept ->string&
			{	
				//First job
				str += " Dani.";
				return str;
			},std::ref(str))
			| execution::next( [](string& str) noexcept -> string&
			{
				//Second job 
				str+= " .How are you?";
				return str;
			})
			| execution::next( [](string& str ) noexcept
			{
				//Third job
				str += "Bye!";
				return str;
			})
			| execution::next( [](string& str ) noexcept
			{
				//Fourth job
				str += "See you!";
				return str;
			})
		);
		if (res.isValid())
		{
			::text::info("Result value = {}",res.value());
			::text::info("Original str = {}",str);
		}
	},0,::tasking::Runnable::_killFalse);
	*/
	th->fireAndForget([ex,&str] () mutable
	{
		auto res = ::tasking::waitForFutureMThread(
			execution::start(ex)
			| execution::inmediate(std::ref(str))
			| execution::next([](string& str) noexcept ->string&
			{	
				//First job
				str += " Dani.";
				return str;
			})
			| execution::next( [](string& str) noexcept -> string&
			{
				//Second job 
				str+= " .How are you?";
				return str;
			})
			| execution::next( [](string& str ) noexcept
			{
				//Third job
				str += "Bye!";
				return str;
			})
			| execution::next( [](string& str ) noexcept
			{
				//Fourth job
				str += "See you!";
				return str;
			})
		);
		if (res.isValid())
		{
			::text::info("Result value = {}",res.value());
			::text::info("Original str = {}",str);
		}
	},0,::tasking::Runnable::_killFalse);

}
void _samples()
{
	text::set_level(text::level::info);
	auto th = ThreadRunnable::create(true);			
	execution::Executor<Runnable> exr(th);
	exr.setOpts({true,false,false});
	parallelism::ThreadPool::ThreadPoolOpts opts;
	auto myPool = make_shared<parallelism::ThreadPool>(opts);
	parallelism::ThreadPool::ExecutionOpts exopts;
	execution::Executor<parallelism::ThreadPool> extp(myPool);
	extp.setOpts({true,true});
	//_sampleBasic(exr);	
//	_sampleBasic(extp);
	_sampleReference(exr);
	text::info("HECHO");
}
//más ejemplos: otro empezando con start y un inmediate; referencias,transferencia a executor, gestion errores, que no siempre sea una lambda, que se usen cosas microhililes.....
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
		auto res1_1 = 
			execution::start(ex)
			| execution::inmediate(TestClass(initVal,test,ll)) 				
			| execution::parallel(
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
			});
		//very simple second job
		auto res1_2 = execution::launch(ex,[]()
		{
			return "pepe";
		});
		//use on_all to launch two "simultaneus" jobs and wait for both
		try
		{
			auto res1 = tasking::waitForFutureMThread( execution::on_all(ex,res1_1,res1_2) );
			int finalVal = std::get<0>(res1.value()).val;
			int expectedVal = initVal + 5+3;
			text::debug("Finish Val = {}",finalVal);
			if (finalVal != expectedVal)
			{
				std::stringstream ss;
				ss << " final value after chain of execution is no correct. Expected "<<expectedVal << " got "<<finalVal;
				test->setFailed(ss.str());
			}
		}catch(std::exception& e)
		{
			text::info("Error = {}",e.what());
		}

		}		
		test->checkOccurrences("TestClass constructor",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);		
		test->checkOccurrences("TestClass copy",0,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);							
		test->checkOccurrences("destructor",test->findTextInBuffer("constructor"),__FILE__,__LINE__);
			
		// 	base form to do the same
		// 	execution::next(
		// 	execution::transfer(
		// 	execution::parallel(
		// 		execution::inmediate(execution::start(ex),TestClass(initVal,test,ll))
		// 	,
		// 	//parallel
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
					

		//now using reference
		test->clearTextBuffer();
		pp.logLevel = ll;
		text::info("Simple functor chaining using reference");
		try
		{
			auto res2 = tasking::waitForFutureMThread(
				execution::launch(ex,[](TestClass& tc) -> TestClass&
				{
					tc.val = 7;
					tasking::Process::wait(1000);
					return tc;
					//throw std::runtime_error("chiquilin");
				},std::ref(pp)) 
				| execution::next([test,ll](TestClass& v) -> TestClass&
				{
					v.val += 10;
					return v;
				}));
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
		}catch(std::exception& e)
		{
			text::info("Error = {}",e.what());
		}
		
		test->checkOccurrences("constructor",0,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		test->checkOccurrences("destructor",test->findTextInBuffer("constructor"),__FILE__,__LINE__);
		//now a little more complex test //TODO: meter al final un throw o algo así
		text::info("A little more complex functor chaining using reference and parallel loops");
		test->clearTextBuffer();
		
		#define LOOP_SIZE 100
		#define INITIAL_VALUE 2
		try
		{
			auto res3 = tasking::waitForFutureMThread(
				execution::start(ex) 
				| execution::inmediate(std::ref(vec)) 
				| execution::next([test,ll](vector<TestClass>& v)->vector<TestClass>&
					{				
						//fill de vector with a simple case for the result to be predecible
						//I don't want out to log the initial constructions, oncly constructons and after this function
						auto t = TestClass(INITIAL_VALUE,test,tests::BaseTest::LogLevel::None,false);
						v.resize(LOOP_SIZE,t);	
						for(auto& elem:v)
						{
							elem.logLevel = ll;
							elem.addToBuffer = true;
						}
						return v;
					}
					)
					| execution::next([](vector<TestClass>& v)->vector<TestClass>&
					{
						for(auto& elem:v)
							++elem.val;						
						return v;	
					})
					| execution::parallel([](vector<TestClass>& v)
					{					
						//multiply by 2 the first half								
						size_t endIdx = v.size()/2;	
						for(size_t i = 0; i < endIdx;++i )
						{
							v[i].val = v[i].val*2.f;	
						}
					},
					[](vector<TestClass>& v)			
					{
						//multiply by 3 the second half
						size_t startIdx = v.size()/2;
						for(size_t i = startIdx; i < v.size();++i )
						{
							v[i].val = v[i].val*3.f;
						}
					}) 
					| execution::loop(
						0,LOOP_SIZE,
						[](int idx,vector<TestClass>& v)
						{
							v[idx].val+=5.f;
						},1
					)
					| execution::next(
						[](vector<TestClass>& v)->vector<TestClass>&
						{															
							for(auto& elem:v)
								++elem.val;		
							return v;				
						}
			));
			stringstream ss;
			ss<<"Valor vector: ";
			for(const auto& v:vec)
				ss << v.val<<' ';
			ss << '\n';
			test->addTextToBuffer(ss.str(),tests::BaseTest::LogLevel::None); 
			//compare result with original vector. Must be the same
			const auto& v = res3.value(); 
			if ( &v != &vec )
				test->setFailed("Both vectors must be the same ");
		}catch(std::exception& e)
		{
			text::info("Error = {}",e.what());
		}
			
		test->checkOccurrences("constructor",0,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		test->checkOccurrences("copy",0,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
	
		test->checkOccurrences(std::to_string((INITIAL_VALUE+1)*2+5+1),vec.size()/2,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		test->checkOccurrences(std::to_string((INITIAL_VALUE+1)*3+5+1),vec.size()/2,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		
		text::info("Same process as the previous without using reference");
		//ss.str(""s); //empty stream
		test->clearTextBuffer();
		try
		{
			auto res4 = tasking::waitForFutureMThread(
				execution::start(ex) 
				| execution::next([test,ll]
				{				
					return vector<TestClass>();
				})
					| execution::next([test,ll](vector<TestClass>& v)
					{				
						//fill de vector with a simple case for the result to be predecible
						//I don't want out to log the initial constructions, oncly constructons and after this function
						auto t = TestClass(INITIAL_VALUE,test,tests::BaseTest::LogLevel::None,false);
						v.resize(LOOP_SIZE,t);	
						for(auto& elem:v)
						{
							elem.logLevel = ll;
							elem.addToBuffer = true;
						}
						return std::move(v); 
					}
					)
					| execution::next([](vector<TestClass>& v)
					{
					
						size_t s = v.size();
						for(auto& elem:v)
							++elem.val;						
						return std::move(v);	
					})
					| execution::parallel([](vector<TestClass>& v)
					{					
						//multiply by 2 the first half			
						size_t endIdx = v.size()/2;	
						for(size_t i = 0; i < endIdx;++i )
						{
							v[i].val = v[i].val*2.f;	
						}
					},
					[](vector<TestClass>& v)			
					{
						//multiply by 3 the second half

						size_t startIdx = v.size()/2;
						for(size_t i = startIdx; i < v.size();++i )
						{
							v[i].val = v[i].val*3.f;
						}
					}) 
					| execution::loop(
						0,LOOP_SIZE,
						[](int idx,vector<TestClass>& v)
						{
							tasking::Process::wait(100);
							v[idx].val+=5.f;
							tasking::Process::wait(2000);
						},1
					)
					| execution::next(
						[](vector<TestClass>& v)
						{					
							for(auto& elem:v)
							{
								++elem.val;		
								tasking::Process::wait(10);
							}
							return std::move(v);				
						})
			);
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
		}catch( std::exception& e)
		{
			text::info("Error = {}",e.what());
		}
		
		test->checkOccurrences("constructor",0,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		test->checkOccurrences("copy",0,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);				

		text::info("Calculate mean of a long vector");
		typedef vector<float> VectorType;
		auto values = std::make_unique<VectorType>();  //create pointer because this is executed in a microthread, so no local variable can be referenced
		//first, create random, big vector in another thread
		//@todo ampliar este ejemplo a un when_all
		try
		{
			auto res5 = tasking::waitForFutureMThread(
			execution::launch(ex,[](VectorType& v)->VectorType&
			{
				//generate random big vector
				int vecSize = std::rand()%1000'000;
				v.resize(vecSize);
				for( size_t i = 0; i < vecSize;++i)
					v[i] = (std::rand()%20)/3.f; //to create a float
				return v;
			},std::ref(*values))
			| execution::parallel_convert<std::tuple<float,float,float,float>>(  //calculate mean in 4 parts @todo ¿cómo podrái devolver este resultado a siguiente funcion?
				[](VectorType& v)
				{
					float mean = 0.f;
					size_t tam = v.size()/4;
					size_t endIdx = tam;
					for(size_t i = 0; i < endIdx;++i)
						mean += v[i];
					mean /= v.size();
					return mean;
				},
				[](VectorType& v)
				{
					float mean = 0.f;
					size_t tam = v.size()/4;
					size_t startIdx = tam;
					size_t endIdx = tam*2;
					for(size_t i = startIdx; i < endIdx;++i)
						mean += v[i];
					mean /= v.size();
					return mean;
				},
				[](VectorType& v)
				{
					float mean = 0.f;
					size_t tam = v.size()/4;
					size_t startIdx = tam*2;
					size_t endIdx = tam*3;
					for(size_t i = startIdx; i < endIdx;++i)
						mean += v[i];
					mean /= v.size();
					return mean;
				},
				[](VectorType& v)
				{
					float mean = 0.f;
					size_t tam = v.size()/4;
					size_t startIdx = tam*3;
					size_t endIdx = v.size();
					for(size_t i = startIdx; i < endIdx;++i)
						mean += v[i];
					mean /= v.size();
					return mean;
				}
			) | execution::next( [](std::tuple<float,float,float,float>& means)
			{
				return (std::get<0>(means)+std::get<1>(means)+std::get<2>(means)+std::get<3>(means));
			}));
			text::info("Mean = {}",res5.value());
		}catch(std::exception& e)
		{
			text::info("Error = {}",e.what());
		}
		
		//ss.str(""s); //empty stream
		test->clearTextBuffer();
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
			_basicTests(extp,th1.get(),test);
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
				case 1001:
					_samples();
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