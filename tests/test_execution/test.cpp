#include "test.h"
#include "test_debug.h"
using test_execution::TestExecution;
#include <tasking/ThreadRunnable.h>
using mel::tasking::ThreadRunnable;
using namespace std;
#include <TestManager.h>
using tests::TestManager;
#include <text/logger.h>
#include <CommandLine.h>
#include <mpl/LinkArgs.h>
#include <mpl/Ref.h>
#include <string>
#include <tasking/Process.h>
using mel::tasking::Process;
#include <tasking/utilities.h>
#include <execution/RunnableExecutor.h>
#include <execution/ThreadPoolExecutor.h>
#include <execution/NaiveInlineExecutor.h>
#include <execution/InlineExecutor.h>
#include <vector>
using std::vector;
#include "test_samples.h"
using namespace mel;


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
static tests::BaseTest* sCurrentTest = nullptr;
struct TestClass
{
	float val;
	bool addToBuffer;
private:
	tests::BaseTest::LogLevel logLevel;
public:

	TestClass(tests::BaseTest::LogLevel ll = tests::BaseTest::LogLevel::Info,bool atb = true):val(1),logLevel(ll),addToBuffer(atb)
	{
		_printTxt("TestClass constructor\n");
	}
	explicit TestClass(float aVal,tests::BaseTest::LogLevel ll = tests::BaseTest::LogLevel::Info,bool atb = true):val(aVal),logLevel(ll),addToBuffer(atb)
	{
		_printTxt("TestClass constructor\n");
	}
	TestClass(const TestClass& ob)
	{
		val = ob.val;
		logLevel = ob.logLevel;
		addToBuffer = ob.addToBuffer;
		_printTxt("TestClass copy constructor\n");
	}
	TestClass(TestClass&& ob)
	{
		val = ob.val;
		ob.val = -1;
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
		logLevel = ob.logLevel;
		addToBuffer = ob.addToBuffer;
		_printTxt("TestClass copy operator=\n");
		return *this;
	}
	TestClass& operator=(TestClass&& ob)
	{
		val = ob.val;
		ob.val = -1;
//		ob.test = nullptr; lo necesito en destructor
		logLevel = ob.logLevel;
		addToBuffer = ob.addToBuffer;
		_printTxt("TestClass move operator=\n");
		return *this;
	}
	void setLogLevel(tests::BaseTest::LogLevel ll)
	{
		logLevel = ll;
	}
	private:
	void _printTxt(const string& str)
	{
		if (addToBuffer && sCurrentTest)
			sCurrentTest->addTextToBuffer(str,logLevel);
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


//basic test for chain of jobs
static bool wasError = false;//for testing
template <class ExecutorType> void _basicTests(ExecutorType ex,ThreadRunnable* th,tests::BaseTest* test)
{
	core::Event event;
	TestClass pp(8);
	vector<TestClass> vec;
	sCurrentTest = test;
	//use a task to make it more complex
	th->fireAndForget([ex,&event,test,&pp,&vec] () mutable
	{
		srand(time(NULL));
		constexpr tests::BaseTest::LogLevel ll = tests::BaseTest::LogLevel::Debug;
		text::info("Simple functor chaining. using operator | from now");
		test->clearTextBuffer();
		constexpr int initVal = 8;		
		{		
			auto th2 = ThreadRunnable::create();					
			execution::Executor<Runnable> ex2(th2);		
			auto res1_1 = 
				// execution::launch(ex,[]()
				// {
				// 	auto r = TestClass(initVal,ll);
				// 	return r;
				// })
				execution::start(ex)
				| mel::execution::inmediate(TestClass(initVal,ll)) 	// move constructor ¿@why 3?
				| mel::execution::parallel(
					[]( TestClass v) noexcept   //one copy and "some" moves ¿why?
					{
						text::debug("Bulk 1");					
						auto prevVal = v.val;
						tasking::Process::wait(1000);
						v.val = prevVal + 5; //should has effect, for testing purposes
					},
					[](const TestClass& v) 
					{
						text::debug("Bulk 2");
						tasking::Process::wait(300);
					})  
				| mel::execution::transfer(ex2)  //transfer execution to a different executor
				| mel::execution::next([test,ll]( const TestClass& v) noexcept
				{
					//text::info("Val = {}",v.value().val);
					const_cast<TestClass&>(v).val+= 3; //very bad practice. It will change passed value, For testing purposes
					//v.val+= 3;
					return v; //@why a lot of moves after here, maybe because of the on_all
				})
				;
			const string strR = "pepe";
			//very simple second job
			auto res1_2 = mel::execution::launch(ex2,[strR]() noexcept
			{
				return strR;
			});
			//use on_all to launch two "simultaneus" jobs and wait for both
			try
			{
				auto res1 = mel::tasking::waitForFutureMThread( mel::execution::on_all(ex,res1_1,res1_2) );
				auto& finalResult = res1.value();
				int finalVal = std::get<0>(finalResult).val;
				int expectedVal = initVal + 3; //because the "forced" const_cast
				text::debug("Finish Val = {}",finalVal);
				std::stringstream ss;
				if (finalVal != expectedVal)
				{
					ss << " final value after chain of execution is no correct. Expected "<<expectedVal << " got "<<finalVal;					
				}
				if (std::get<1>(finalResult) != strR)
				{
					ss << " final second element in value after chain of execution is no correct. Expected "<< strR << " got "<<std::get<1>(finalResult);					
				}
				if ( ss.rdbuf()->in_avail() != 0)
					test->setFailed(ss.str());
			}catch(std::exception& e)
			{
				text::info("Error = {}",e.what());
			}
		}		
		test->checkOccurrences("TestClass constructor",2,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info); //initial constructor from inmedaite, and default constructor in tuple elements
		test->checkOccurrences("TestClass copy",2,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);							
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
		pp.setLogLevel(ll);
		{		
			text::info("Simple functor chaining using reference");
			try
			{
				auto res2 = mel::tasking::waitForFutureMThread(
					execution::launch(ex,[](TestClass& tc) noexcept -> TestClass& 
					{
						tc.val = 7;
						tasking::Process::wait(1000);
						return tc;
						//throw std::runtime_error("chiquilin");
					},std::ref(pp)) 
					| mel::execution::next([test,ll](TestClass& v) noexcept-> TestClass&
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
			auto res3 = mel::tasking::waitForFutureMThread(
				execution::start(ex) 
				| mel::execution::inmediate(std::ref(vec)) 
				| mel::execution::next([test,ll](vector<TestClass>& v) noexcept ->vector<TestClass>&
					{				
						//fill de vector with a simple case for the result to be predecible
						//I don't want out to log the initial constructions, oncly constructons and after this function
						auto t = TestClass(INITIAL_VALUE,tests::BaseTest::LogLevel::None,false);
						v.resize(LOOP_SIZE,t);	
						for(auto& elem:v)
						{
							elem.setLogLevel(ll);
							elem.addToBuffer = true;
						}
						return v;
					}
					)
					| mel::execution::next([](vector<TestClass>& v) noexcept ->vector<TestClass>&
					{
						for(auto& elem:v)
							++elem.val;						
						return v;	
					})
					| mel::execution::parallel(
						[](vector<TestClass>& v) noexcept
						{					
							//multiply by 2 the first half								
							size_t endIdx = v.size()/2;	
							for(size_t i = 0; i < endIdx;++i )
							{
								v[i].val = v[i].val*2.f;	
							}
						},
						[](vector<TestClass>& v) noexcept
						{
							//multiply by 3 the second half
							size_t startIdx = v.size()/2;
							for(size_t i = startIdx; i < v.size();++i )
							{
								v[i].val = v[i].val*3.f;
							}
						}
					) 
					| mel::execution::loop(
						0,LOOP_SIZE,
						[](int idx,vector<TestClass>& v) noexcept
						{
							v[idx].val+=5.f;
						},1
					)
					| mel::execution::next(
						[](vector<TestClass>& v) noexcept->vector<TestClass>&
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
		
		/*
		este método ya no tiene snetido ya que al no pasar referencia no puede modificar entrada
		text::info("Same process as the previous without using reference");
		//this method will produce a lot of copies, movements		
		test->clearTextBuffer();
		try
		{			
			if ( rand()%10 < 5 )
				wasError = true;
			else	
				wasError = false;
			auto res4 = mel::tasking::waitForFutureMThread(
				execution::start(ex) 
				| mel::execution::next([test,ll]() noexcept
				{				
					auto t = TestClass(INITIAL_VALUE,tests::BaseTest::LogLevel::None,false);
					vector<TestClass> v(LOOP_SIZE,t);
					for(auto& elem:v)
					{
						elem.setLogLevel(ll);
						elem.addToBuffer = true;
					}
					return std::move(v); 
				})
					| mel::execution::next([](vector<TestClass> v) noexcept
					{
					//@todo ahora estos ejemplos no vale, porque no es referencia
						size_t s = v.size();
						for(auto& elem:v)
							++elem.val;						
						return std::move(v);	
					})
					ahora el parallel no tiene sentido porque no puede modificar la entrada
					si uso un convert tiene que devovler tupla
					| mel::execution::parallel(
						[](const vector<TestClass>& v) 
						{		
							//randomly throw exception												
							if ( wasError )
							{
								throw std::runtime_error("random error");
							}							
							//multiply by 2 the first half			
							size_t endIdx = v.size()/2;	
							for(size_t i = 0; i < endIdx;++i )
							{
								//v[i].val = v[i].val*2.f;	
							}
						},
						[]( const vector<TestClass>& v) noexcept
						{
							//multiply by 3 the second half
							size_t startIdx = v.size()/2;
							for(size_t i = startIdx; i < v.size();++i )
							{					
								//@todo el problema ahora es que el bulk no vale para mucho, no??			
								v[i].val = v[i].val*3.f;
							}
						}
					) 
					| mel::execution::loop(
						0,LOOP_SIZE,
						[](int idx,const vector<TestClass>& v) noexcept
						{
							tasking::Process::wait(100);
							v[idx].val+=5.f;
							tasking::Process::wait(2000);
						},1
					)
					|execution::catchError([](std::exception_ptr err) 
					{
						try
						{
							std::rethrow_exception(err);
						}catch(std::exception& e)
						{
							sCurrentTest->addTextToBuffer("captured error "s+e.what(),tests::BaseTest::LogLevel::None);
						}
						catch(...)
						{
							sCurrentTest->addTextToBuffer("captured error unknkown"s,tests::BaseTest::LogLevel::None);
						}
						
						//this job capture error is any was thrown in previous jobs, and create a new vector for the next job						
						vector<TestClass> newVector(10,TestClass(1,tests::BaseTest::LogLevel::None,false));
						return newVector;
					})
					| mel::execution::next(
						[](const vector<TestClass>& v) noexcept
						{					
							for(auto& elem:v)
							{
								//++elem.val;		
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
			if (!wasError)
			{
				test->checkOccurrences(std::to_string((INITIAL_VALUE+1)*2+5+1),res4.value().size()/2,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
				test->checkOccurrences(std::to_string((INITIAL_VALUE+1)*3+5+1),res4.value().size()/2,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
			}else
			{
				//in case of error, en new vector of size 10 is created, with values initialized to 1, and later incremented in last job
				test->checkOccurrences("2",10,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
			}
		}catch( std::exception& e)
		{
			text::info("Error = {}",e.what());
		}
		
		test->checkOccurrences("constructor",0,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		test->checkOccurrences("copy",0,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);				
*/
		
		
		//ss.str(""s); //empty stream
		test->clearTextBuffer();
		event.set();
	});
	event.wait();
}
template <class ExecutorType> void _testMeanVector(::mel::execution::ExFuture<ExecutorType,vector<double>> fut,string title,tests::BaseTest* test)
{
	typedef vector<double> VectorType;
	try
	{
		Timer timer;
		uint64_t t0 = timer.getMilliseconds();

		auto res5 = mel::core::waitForFutureThread(
		/*execution::launch(ex,[](VectorType& v)->VectorType&
		{				
			//int vecSize = std::rand()%1000'000; //generate random big vector
			constexpr int vecSize = 2000'000;
			v.resize(vecSize);
			for( size_t i = 0; i < vecSize;++i)
				v[i] = (std::rand()%20)/3.f; //to create a float
			return v;
		},std::ref(*values))
		*/
		fut
		//| mel::execution::parallel_convert<std::tuple<double,double,double,double>>(  //calculate mean in 4 parts @todo ¿cómo podrái devolver este resultado a siguiente funcion?
		| mel::execution::parallel_convert(  //calculate mean in 4 parts
			[](const VectorType& v) noexcept
			{
				double mean = 0.f;
				size_t tam = v.size()/4;
				size_t endIdx = tam;
				for(size_t i = 0; i < endIdx;++i)
					mean += v[i];
				mean /= v.size();
				return mean;
			},
			[](const VectorType& v) noexcept
			{
				double mean = 0.f;
				size_t tam = v.size()/4;
				size_t startIdx = tam;
				size_t endIdx = tam*2;
				for(size_t i = startIdx; i < endIdx;++i)
					mean += v[i];
				mean /= v.size();
				return mean;
			},
			[](const VectorType& v) noexcept
			{
				double mean = 0.f;
				size_t tam = v.size()/4;
				size_t startIdx = tam*2;
				size_t endIdx = tam*3;
				for(size_t i = startIdx; i < endIdx;++i)
					mean += v[i];
				mean /= v.size();
				return mean;
			},
			[](const VectorType& v) noexcept
			{
				double mean = 0.f;
				size_t tam = v.size()/4;
				size_t startIdx = tam*3;
				size_t endIdx = v.size();
				for(size_t i = startIdx; i < endIdx;++i)
					mean += v[i];
				mean /= v.size();
				return mean;
			}
		)
		| mel::execution::next( [](const std::tuple<double,double,double,double>& means)
			{
				return (std::get<0>(means)+std::get<1>(means)+std::get<2>(means)+std::get<3>(means));
			}
		)
		);
		uint64_t t1 = timer.getMilliseconds();
		text::info("Mean = {}. Time spent = {} seconds",res5.value(),(float)((t1-t0)/1000.f));
		tests::BaseTest::addMeasurement(title+" time:",(float)((t1-t0)/1000.f));
	}catch(std::exception& e)
	{
		text::info("Error = {}",e.what());
	}
}
template <class ExecutorType> void _testMeanVectorLoop(::mel::execution::ExFuture<ExecutorType,vector<double>> fut,string title,tests::BaseTest* test)
{
	using namespace mel::execution;
	typedef vector<double> VectorType;
	try
	{
		Timer timer;
		uint64_t t0 = timer.getMilliseconds();
		int numParts = ExecutorTraits<Executor<ExecutorType>>::has_parallelism?mel::core::getNumProcessors():1;
		vector<double> means(numParts);
		auto res5 = mel::core::waitForFutureThread(
		fut
		| mel::execution::loop(
						0,(int)numParts,
						[&means,numParts](int idx,const VectorType& v) noexcept
						{
							double mean = 0.f;
							size_t tam = v.size()/numParts;  //size of each sub-vector
							size_t startIdx = idx*tam;
							size_t endIdx;
							if ( idx == numParts-1)
								endIdx = v.size();
							else
								endIdx = startIdx+tam;
							for(size_t i = startIdx; i < endIdx;++i)
								mean += v[i];
							mean /= v.size();
							means[idx] = mean;
						},1
					)
		 | mel::execution::next( [&means](const VectorType&)
		 {
			double mean = 0.0;
			for(size_t i = 0; i < means.size();++i)
				mean+=means[i];
			return mean; 
		 })		
		);
		uint64_t t1 = timer.getMilliseconds();
		text::info("Using loop. Mean = {}. Time spent = {} seconds",res5.value(),(float)((t1-t0)/1000.f));
		tests::BaseTest::addMeasurement(title+" time:",(float)((t1-t0)/1000.f));
	}catch(std::exception& e)
	{
		text::info("Error = {}",e.what());
	}
}

//basic test for launching task in execution agents
int _testLaunch( tests::BaseTest* test)
{
	int result = 0;		
	mel::text::info("Test execution: launch");
	{
		{
			auto th1 = ThreadRunnable::create(true);			
			execution::Executor<Runnable> exr(th1);
			exr.setOpts({true,false});
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
		{
			auto th1 = ThreadRunnable::create(true);
			execution::NaiveInlineExecutor ex;
			text::info("\n\tBasicTests with NaiveInlineExecutor");
			_basicTests(ex,th1.get(),test);
			text::info("\n\tFinished BasicTests with NaiveInlineExecutor");
		}
		/*
		InlinExecutor doesn't do any copy because it's customized for this specific case
		{
			auto th1 = ThreadRunnable::create(true);
			execution::InlineExecutor ex;
			text::info("\n\tBasicTests with InlineExecutor");
			_basicTests(ex,th1.get(),test);
			text::info("\n\tFinished BasicTests with InlineExecutor");
		}
		*/
	}	
	return result;
}

// "move" tests are removed because they depend greately on compiler and debug vs release
template <class ExecutorType> void _testCapturesHelper(ExecutorType ex,ThreadRunnable* th,tests::BaseTest* test)
{
	#define INIT_VALUE 2
	sCurrentTest = test;
	tests::BaseTest::LogLevel ll = tests::BaseTest::LogLevel::Info;
	{
		mel::text::info("Test Launch lambda rvalue ref");
		//first test: Passing lambda as rvalue reference and capturing object by copy
		TestClass pp(INIT_VALUE,ll);
		int cont = INIT_VALUE;
		auto res = mel::core::waitForFutureThread(
			 execution::launch(ex,
			 	[pp,cont]()
				{
					mel::text::info("Launch before wait. pp.val = {}, cont = {}",pp.val,cont);
					if ( cont != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "Cont value is "<<cont<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					if ( pp.val != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "pp.val value is "<<pp.val<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					if ( mel::execution::ExecutorTraits<ExecutorType>::has_microthreading)
						mel::tasking::Process::wait(2000);
					else
						mel::text::warn("Current executor hasn't microthread capabilities");
					mel::text::info("Launch after wait. pp.val = {}, cont = {}",pp.val,cont);
					return "HOLA"s;		
				}
			 )
		);	
		//mel::text::info("Value = {}",res.value());
		test->checkOccurrences("TestClass constructor",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		test->checkOccurrences("TestClass copy",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		//test->checkOccurrences("TestClass move",2,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);  //moved when calling launch + move to execute							
		// test->checkOccurrences("destructor",test->findTextInBuffer("constructor"),__FILE__,__LINE__);
	}
	{
		sCurrentTest->clearTextBuffer();
		mel::text::info("Test Launch lambda lvalue ref");
		//second test: Passing lambda as lvalue reference and capturing object by copy
		TestClass pp(INIT_VALUE,ll);
		int cont = INIT_VALUE;
		auto lmb = [pp,cont]()
				{
					mel::text::info("Launch before wait. pp.val = {}, cont = {}",pp.val,cont);
					if ( cont != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "Cont value is "<<cont<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					if ( pp.val != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "pp.val value is "<<pp.val<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					if ( mel::execution::ExecutorTraits<ExecutorType>::has_microthreading)
						mel::tasking::Process::wait(2000);
					else
						mel::text::warn("Current executor hasn't microthread capabilities");
					mel::text::info("Launch after wait. pp.val = {}, cont = {}",pp.val,cont);
					return "HOLA"s;		
				};
		auto res = mel::core::waitForFutureThread(
			 execution::launch(ex,lmb)
		);	
		mel::text::info("Value = {}",res.value());
		test->checkOccurrences("TestClass constructor",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		test->checkOccurrences("TestClass copy",2,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info); //copy in capture + copy lamda
	//	test->checkOccurrences("TestClass move",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info); //moved when executed							
	}
	//test next
	{
		sCurrentTest->clearTextBuffer();
		mel::text::info("Test Next lambda rvalue ref");
		//first test: Passing lambda as rvalue reference and capturing object by copy
		TestClass pp(INIT_VALUE,ll);
		int cont = INIT_VALUE;
		auto res = mel::core::waitForFutureThread(
			execution::launch( ex,[]() noexcept
				{
					return "Dani"s;
				}
			)
			| execution::next( [pp,cont](const string& s)
				{
					mel::text::info("Next before wait. pp.val = {}, cont = {}",pp.val,cont);
					if ( cont != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "Cont value is "<<cont<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					if ( pp.val != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "pp.val value is "<<pp.val<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					if ( mel::execution::ExecutorTraits<ExecutorType>::has_microthreading)
						mel::tasking::Process::wait(2000);
					else
						mel::text::warn("Current executor hasn't microthread capabilities");
					mel::text::info("Next after wait. pp.val = {}, cont = {}",pp.val,cont);
					return s;
				}
			) 			
		
		);	
	//	mel::text::info("Value = {}",res.value());
		test->checkOccurrences("TestClass constructor",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		test->checkOccurrences("TestClass copy",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info); //copy in capture
	//	test->checkOccurrences("TestClass move",10,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info); //moved when executed			
	}
	{
		sCurrentTest->clearTextBuffer();
		mel::text::info("Test next lambda lvalue ref");
		//second test: Passing lambda as lvalue reference and capturing object by copy
		TestClass pp(INIT_VALUE,ll);
		int cont = INIT_VALUE;
		auto lmb = [pp,cont](const string& s)
				{
					mel::text::info("Next before wait. pp.val = {}, cont = {}",pp.val,cont);
					if ( cont != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "Cont value is "<<cont<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					if ( pp.val != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "pp.val value is "<<pp.val<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					if ( mel::execution::ExecutorTraits<ExecutorType>::has_microthreading)
						mel::tasking::Process::wait(2000);
					else
						mel::text::warn("Current executor hasn't microthread capabilities");
					mel::text::info("Next after wait. pp.val = {} cont = {}",pp.val,cont);
					return s;
				};		
		auto res = mel::core::waitForFutureThread(
			execution::launch( ex,[]() noexcept
				{
					return "Dani"s;
				}
			)
			| execution::next( 
				lmb
			) 
		
		);	
		mel::text::info("Value = {}",res.value());
		test->checkOccurrences("TestClass constructor",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		test->checkOccurrences("TestClass copy",2,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info); //copy in capture
//		test->checkOccurrences("TestClass move",7,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info); //moved when executed	
	}
	//parallel
	{
		sCurrentTest->clearTextBuffer();
		mel::text::info("Test Parallel lambda rvalue ref");
		TestClass pp(INIT_VALUE,ll);
		int cont = INIT_VALUE;
		//first test: Passing lambda as rvalue reference and capturing object by copy
		auto res = mel::core::waitForFutureThread(
			execution::launch( ex,[]() noexcept
				{
					return "Dani"s;
				}
			)
			| execution::parallel( 
				[cont](const string& s)
				{
					mel::text::info("Parallel t1. cont = {}",cont);
					if ( cont != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "Cont value is "<<cont<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
				},
				[pp,cont](const string& s)
				{
					mel::text::info("Parallel t2. pp.val = {} cont = {}",pp.val,cont);
					if ( cont != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "Cont value is "<<cont<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					if ( pp.val != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "pp.val value is "<<pp.val<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
				}
			)
		);	
		mel::text::info("Value = {}",res.value());
		test->checkOccurrences("TestClass constructor",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
		test->checkOccurrences("TestClass copy",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info); //copy in capture
	//	test->checkOccurrences("TestClass move",8,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info); //moved when executed	
	}
	{		
		sCurrentTest->clearTextBuffer();
		mel::text::info("Test Parallel lambda lvalue ref");
		TestClass pp(INIT_VALUE,ll);
		int cont = INIT_VALUE;
		auto lmb = [pp,cont](const string& s)
				{
					mel::text::info("Parallel lmb. pp.val = {} cont = {}",pp.val,cont);
					if ( cont != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "Cont value is "<<cont<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					if ( pp.val != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "pp.val value is "<<pp.val<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
				};

		//second test: Passing lambda as lvalue reference and capturing object by copy
		auto res = mel::core::waitForFutureThread(
			execution::launch( ex,[]() noexcept
				{
					return "Dani"s;
				}
			)
			| execution::parallel( 
				[cont](const string& s)
				{
					mel::text::info("Parallel t1. cont = {}",cont);
					if ( cont != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "Cont value is "<<cont<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
				}
				,
				lmb
			)
		);	
		mel::text::info("Value = {}",res.value());
		test->checkOccurrences("TestClass constructor",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info); //initial constructor from inmedaite, and default constructor in tuple elements
		test->checkOccurrences("TestClass copy",2,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);							
		// test->checkOccurrences("destructor",test->findTextInBuffer("constructor"),__FILE__,__LINE__);
	}
	//parallel_convert rvalue ref
	{
		sCurrentTest->clearTextBuffer();
		mel::text::info("Test Parallel_convert lambda rvalue ref");
		TestClass pp(INIT_VALUE,ll);
		int cont = INIT_VALUE;
		//first test: Passing lambda as rvalue reference and capturing object by copy
		auto res = mel::core::waitForFutureThread(
			execution::launch( ex,[]() noexcept
				{
					return "Dani"s;
				}
			)
			| execution::parallel_convert( 
				[cont](const string& s)
				{
					mel::text::info("Parallel_convert t1. cont = {}",cont);
					if ( cont != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "Cont value is "<<cont<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					return "hola"s;
				},
				[pp,cont](const string& s)
				{
					mel::text::info("Parallel_convert t2. pp.val = {} cont = {}",pp.val,cont);
					if ( cont != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "Cont value is "<<cont<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					if ( pp.val != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "pp.val value is "<<pp.val<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					return 9.6f;
				}
			)
		);	
		mel::text::info("Value = ({} {})",std::get<0>(res.value()),std::get<1>(res.value()));
		test->checkOccurrences("TestClass constructor",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info); //initial constructor from inmedaite, and default constructor in tuple elements
		test->checkOccurrences("TestClass copy",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);							
		// test->checkOccurrences("destructor",test->findTextInBuffer("constructor"),__FILE__,__LINE__);
	}
	//parallel_convert lvalue ref
	{		
		sCurrentTest->clearTextBuffer();
		mel::text::info("Test Parallel_convert lambda lvalue ref");
		TestClass pp(INIT_VALUE,ll);
		int cont = INIT_VALUE;
		auto lmb = [pp,cont](const string& s)
				{
					mel::text::info("Parallel_convert lmb. pp.val = {} cont = {}",pp.val,cont);
					if ( cont != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "Cont value is "<<cont<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					if ( pp.val != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "pp.val value is "<<pp.val<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					return 9.6f;
				};

		//second test: Passing lambda as lvalue reference and capturing object by copy
		auto res = mel::core::waitForFutureThread(
			execution::launch( ex,[]() noexcept
				{
					return "Dani"s;
				}
			)
			| execution::parallel_convert( 
				[cont](const string& s)
				{
					mel::text::info("Parallel t1. cont = {}",cont);
					if ( cont != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "Cont value is "<<cont<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					return "hola"s;
				},
				lmb
			)
		);	
		mel::text::info("Value = ({} {})",std::get<0>(res.value()),std::get<1>(res.value()));
		test->checkOccurrences("TestClass constructor",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info); //initial constructor from inmedaite, and default constructor in tuple elements
		test->checkOccurrences("TestClass copy",2,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);							
		// test->checkOccurrences("destructor",test->findTextInBuffer("constructor"),__FILE__,__LINE__);
	}
	//loop
	{
		sCurrentTest->clearTextBuffer();
		mel::text::info("Test Loop lambda rvalue ref");
		TestClass pp(INIT_VALUE,ll);
		int cont = INIT_VALUE;
		//first test: Passing lambda as rvalue reference and capturing object by copy
		auto res = mel::core::waitForFutureThread(
			execution::launch( ex,[]() noexcept
				{
					return "Dani"s;
				}
			)
			| execution::loop( 0,10,
				[cont,pp](int idx,const string& s) noexcept
				{
					if ( cont != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "Cont value is "<<cont<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					if ( pp.val != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "pp.val value is "<<pp.val<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
				}
			)
		);	
		mel::text::info("Value = {}",res.value());
		test->checkOccurrences("TestClass constructor",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info); //initial constructor from inmedaite, and default constructor in tuple elements
		test->checkOccurrences("TestClass copy",11,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info); //each iteration means a copy of the lambda
		// test->checkOccurrences("destructor",test->findTextInBuffer("constructor"),__FILE__,__LINE__);
	}
	//loop
	{
		sCurrentTest->clearTextBuffer();
		mel::text::info("Test Loop lambda lvalue ref");
		TestClass pp(INIT_VALUE,ll);
		int cont = INIT_VALUE;
		auto lmb = [cont,pp](int idx,const string& s) noexcept
				{
					//mel::text::info("loop cont = {}",cont);
					if ( cont != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "Cont value is "<<cont<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
					if ( pp.val != INIT_VALUE ) 
					{
						stringstream ss;
						ss << "pp.val value is "<<pp.val<<" but should be "<<INIT_VALUE<<'\n';
						sCurrentTest->setFailed(ss.str());
					}
				};		
		
		auto res = mel::core::waitForFutureThread(
			execution::launch( ex,[]() noexcept
				{
					return "Dani"s;
				}
			)
			| execution::loop( 0,10,
				lmb
			)
		);	
		mel::text::info("Value = {}",res.value());
		test->checkOccurrences("TestClass constructor",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info); //initial constructor from inmedaite, and default constructor in tuple elements
		test->checkOccurrences("TestClass copy",3,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);							
		// test->checkOccurrences("destructor",test->findTextInBuffer("constructor"),__FILE__,__LINE__);
	}
}
//test copies and behaviour in lambda captures
int _testCaptures( tests::BaseTest* test)
{
	int result = 0;
	{
		auto th1 = ThreadRunnable::create(true);			
		execution::Executor<Runnable> exr(th1);
		exr.setOpts({true,false});
		text::info("\n\t_testCaptures with RunnableExecutor");
		_testCapturesHelper(exr,th1.get(),test);
		text::info("\n\tFinished _testCaptures with RunnableExecutor");
	}
	{
		auto th1 = ThreadRunnable::create(true);						
		parallelism::ThreadPool::ThreadPoolOpts opts;
		auto myPool = make_shared<parallelism::ThreadPool>(opts);
		parallelism::ThreadPool::ExecutionOpts exopts;
		execution::Executor<parallelism::ThreadPool> extp(myPool);
		extp.setOpts({true,true});
		text::info("\n\t_testCaptures with ThreadPoolExecutor");
		_testCapturesHelper(extp,th1.get(),test);
		text::info("\n\tFinished _testCaptures with ThreadPoolExecutor");
	}
	{
		auto th1 = ThreadRunnable::create(true);
		execution::NaiveInlineExecutor ex;
		text::info("\n\t_testCaptures with NaiveInlineExecutor");
		_testCapturesHelper(ex,th1.get(),test);
		text::info("\n\tFinished _testCaptures with NaiveInlineExecutor");
	}
	return result;

}
int _testAdvanceSample(tests::BaseTest* test)
{
	int result = 0;	
	text::info("Calculate mean of a long vector");

	Runnable::RunnableCreationOptions opts;
	opts.schedulerOpts = ProcessScheduler::LockFreeOptions{};
	auto th1 = ThreadRunnable::create(true,opts);
	execution::Executor<Runnable> exr(th1);
	parallelism::ThreadPool::ThreadPoolOpts tpopts;
	tpopts.threadOpts.schedulerOpts = ProcessScheduler::LockFreeOptions{};
	auto myPool = make_shared<parallelism::ThreadPool>(tpopts);
	parallelism::ThreadPool::ExecutionOpts exopts;
	execution::Executor<parallelism::ThreadPool> extp(myPool);
	extp.setOpts({true,true});
	typedef vector<double> VectorType;
	auto initFut = execution::launch(exr,[]()
	{				
		//int vecSize = std::rand()%1000'000; //generate random big vector
		constexpr int vecSize = 100'000'000;
		VectorType v(vecSize);
		for( size_t i = 0; i < vecSize;++i)
			v[i] = (std::rand()%20)/3.0; //to create a double
		return v;
	});
	core::waitForFutureThread(initFut); //wait for completion of vector creation to not interfere in time measurement
	auto initRes = core::waitForFutureThread<mel::core::WaitErrorNoException>( initFut );
	Timer timer;
	uint64_t t0 = timer.getMilliseconds();
	
	//plain version
	text::info("vector mean: plain way");
	double mean = 0.0;
	{
		auto& v = initRes.value();
		size_t endIdx = v.size();
		for(size_t i = 0; i < endIdx;++i)
			mean+=v[i];
		mean/=v.size();	
	}	
	uint64_t t1 = timer.getMilliseconds();
	mel::text::info("Mean = {}. Time spent = {}",mean,(float)((t1-t0)/1000.f));	
	tests::BaseTest::addMeasurement("vector mean: plain way time:",(float)((t1-t0)/1000.f));
	{		
		parallelism::ThreadPool::ThreadPoolOpts opts;
		opts.threadOpts.schedulerOpts = ProcessScheduler::LockFreeOptions{};
		auto myPool = make_shared<parallelism::ThreadPool>(opts);
		parallelism::ThreadPool::ExecutionOpts exopts;
		execution::Executor<parallelism::ThreadPool> extp(myPool);
		extp.setOpts({true,true});
		text::info("vector mean: ThreadPoolExecutor");
		_testMeanVector( execution::transfer(initFut,extp),"vector mean: ThreadPoolExecutor",test);
	}
	{
		exr.setOpts({true,false});
		text::info("vector mean: RunnableExecutor");		
		_testMeanVector(execution::transfer(initFut,exr),"vector mean: RunnableExecutor",test); //the transfer is not neccesary because initFut is launched in exr, but jsut in case it changes
	}
	{		
		//en GCC 9 esto no compila, tiene algún bug en resolucion de overloads con templates
		execution::InlineExecutor ex;		
		//execution::NaiveInlineExecutor ex;		
		text::info("vector mean: InlineExecutor");		
		_testMeanVector( execution::transfer(initFut,ex),"vector mean: InlineExecutor",test);
	}
	//now testing different algorithm using execution ::loop
	{
		parallelism::ThreadPool::ThreadPoolOpts opts;
		opts.threadOpts.schedulerOpts = ProcessScheduler::LockFreeOptions{};
		auto myPool = make_shared<parallelism::ThreadPool>(opts);
		parallelism::ThreadPool::ExecutionOpts exopts;
		execution::Executor<parallelism::ThreadPool> extp(myPool);
		extp.setOpts({true,true});
		text::info("vector mean(loop): ThreadPoolExecutor");
		_testMeanVectorLoop( execution::transfer(initFut,extp),"vector mean(loop): ThreadPoolExecutor",test);		
	}
	{
		exr.setOpts({true,false});
		text::info("vector mean(loop): RunnableExecutor");		
		_testMeanVectorLoop(execution::transfer(initFut,exr),"vector mean(loop): RunnableExecutor",test);
	}

	//comparar resultados?
	return result;
}
#ifdef NDEBUG
#define DEFAULT_LOOPSIZE 500'000
#else
#define DEFAULT_LOOPSIZE 100'000
#endif
//don't want to optimize this function because i want to measure in same conditions
//for MSVC
#ifdef _MSC_VER
#pragma optimize("",off)   
#endif
template <class F> void _measureTest(string txt,F f, size_t iters,size_t loopSize) OPTIMIZE_FLAGS;
template <class F> void _measureTest(string txt,F f, size_t iters,size_t loopSize) 
{
	int mean = 0;	
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
//gcc and clang need different order in atributes
#if !defined(MEL_X64_GCC) && !defined(MEL_X86_GCC)
auto gTestFunc=[](int idx) OPTIMIZE_FLAGS noexcept 
#else 
auto gTestFunc=[](int idx) noexcept OPTIMIZE_FLAGS  
#endif
	{
		//text::debug("iteracion pre {}",idx);
		::tasking::Process::switchProcess(true);
		//text::debug("iteracion post {}",idx);
	};
#ifdef _MSC_VER
#pragma optimize("",on)   
#endif	
// loop test to measure performance
int _testFor(tests::BaseTest* test)
{
	int result = 0;
	mel::text::info("Test execution: loop");
	// mel::text::set_level(text::level::info);	
    int loopSize = DEFAULT_LOOPSIZE;
	constexpr int iters = 20;
    //check for loopsize options, if given
	auto opt = tests::CommandLine::getSingleton().getOption("ls");
	if ( opt != nullopt) {
		loopSize = std::stol(opt.value());
	}

	std::stringstream cpuinfo;
	cpuinfo<<"NumCores:"<< ::mel::core::getNumProcessors();
	tests::BaseTest::addMeasurement("CPU Info",cpuinfo.str());

	//basic test with a plain Thread to get a base reference
	/*qué probar?
	una prueba fijo es con el measurement test para ver la creacion
	otra un bucle a pelo */

	_measureTest("Plain thread, only one",[]
	{	
		auto plainThread = std::make_unique<Thread>(
			[] () OPTIMIZE_FLAGS 
			{
				for(volatile int i = 0; i < 100000000;++i);					
				//text::info("Finish plain thread");
			}
		);
		plainThread = nullptr;
	},1,0);
	_measureTest("Plain thread, create/destroy",[]
	{	
		auto plainThread = std::make_unique<Thread>(
			[]() OPTIMIZE_FLAGS 
			{
				for(volatile int i = 0; i < 100000000;++i);
					//text::debug("Plain Thread");
				//text::info("Finish plain thread");
			}
		);
		plainThread = nullptr;
	},iters,0);

	Runnable::RunnableCreationOptions blockingOpts;
	blockingOpts.schedulerOpts = mel::tasking::ProcessScheduler::BlockingOptions{};
	Runnable::RunnableCreationOptions lockFreeOpts;
	lockFreeOpts.schedulerOpts = mel::tasking::ProcessScheduler::LockFreeOptions{10000,4}; //big initial buffer
	_measureTest("Runnable executor with independent tasks",
		[loopSize,opts = blockingOpts]
		{	
			auto th1 = ThreadRunnable::create(true,opts);
			execution::Executor<Runnable> ex(th1);			
			execution::RunnableExecutorOpts exopts;
			exopts.independentTasks = true;
			ex.setOpts(exopts);
			const int idx0 = 0;
			core::waitForFutureThread(execution::loop(execution::start(ex),idx0,loopSize,gTestFunc,1));
			//text::debug("hecho");
			
		},iters,loopSize
	);
	_measureTest("Runnable executor with independent tasks, Lock-free",
		[loopSize,opts = lockFreeOpts]()
		{	
			auto th1 = ThreadRunnable::create(true,opts);
			execution::Executor<Runnable> ex(th1);			
			execution::RunnableExecutorOpts exOpts;
			exOpts.independentTasks = true;
			ex.setOpts(exOpts);
			const int idx0 = 0;
			core::waitForFutureThread(execution::loop(execution::start(ex),idx0,loopSize,gTestFunc,1));
			//text::debug("hecho");
			
		},iters,loopSize
	);

	_measureTest("Runnable executor with independent tasks pausing thread",
		[loopSize,opts = blockingOpts]() mutable
		{
			auto th1 = ThreadRunnable::create(false,opts);
			execution::Executor<Runnable> ex(th1);	
			execution::RunnableExecutorOpts exOpts;
			exOpts.independentTasks = true;
			ex.setOpts(exOpts);
			int idx0 = 0;
			auto r = mel::execution::loop(execution::start(ex),idx0,loopSize,gTestFunc,1);
			th1->resume();
			core::waitForFutureThread(r);
			th1->suspend();
		},iters,loopSize
	);	
	_measureTest("Runnable executor WITHOUT independent tasks",
		[loopSize,opts = blockingOpts]()
		{
			auto th1 = ThreadRunnable::create(true,opts);
			execution::Executor<Runnable> ex(th1);	
			execution::RunnableExecutorOpts exOpts;
			exOpts.independentTasks = false;
			ex.setOpts(exOpts);
			const int idx0 = 0;
			core::waitForFutureThread(execution::loop(execution::start(ex),idx0,loopSize,gTestFunc,1));
		},iters,loopSize
	);
	_measureTest("ThreadPool executor, grouped tasks",
		[loopSize]()
		{
			parallelism::ThreadPool::ThreadPoolOpts opts;
			opts.threadOpts.schedulerOpts = ProcessScheduler::BlockingOptions{};
			auto myPool = make_shared<parallelism::ThreadPool>(opts);
			execution::Executor<parallelism::ThreadPool> extp(myPool);
			execution::ThreadPoolExecutorOpts exOpts;
			exOpts.independentTasks = false;
			extp.setOpts(exOpts);
			const int idx0 = 0;
			core::waitForFutureThread(execution::loop(execution::start(extp),idx0,loopSize,gTestFunc,1));
		},iters,loopSize
	);
	_measureTest("ThreadPool executor, grouped tasks, lock-free",
		[loopSize]()
		{
			parallelism::ThreadPool::ThreadPoolOpts opts;
			opts.threadOpts.schedulerOpts = ProcessScheduler::LockFreeOptions{};
			auto myPool = make_shared<parallelism::ThreadPool>(opts);
			execution::Executor<parallelism::ThreadPool> extp(myPool);
			execution::ThreadPoolExecutorOpts exOpts;
			exOpts.independentTasks = false;
			extp.setOpts(exOpts);
			const int idx0 = 0;
			core::waitForFutureThread(execution::loop(execution::start(extp),idx0,loopSize,gTestFunc,1));
		},iters,loopSize
	);
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
					samples();
					break;					
				case 0:
					result = _testLaunch(this);
					break;
				case 1:
					result = _testFor(this);
					break;				
				case 2:
					result = _testAdvanceSample(this);
					break;
				case 3:
					result = _testCaptures(this);
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
	_testAdvanceSample(this);
	_testFor(this);
	_testCaptures(this);
	return 0;
}