#include "test.h"
using namespace test_threading;
#include <iostream>
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
#include "future_tests.h"
#include <tasking/utilities.h>
#include <array>

const std::string TestThreading::TEST_NAME = "threading";
/**
 * @todo pensar en test neceasrios:
 *  - mono hilo + microhilos
 *  - multihilo +`microhilos
 *  - futures y demas
 * 
 * @return int 
 */

class CustomProcessType : public GenericProcess
{
	public:

	//testing custom operator new. Now do the default but could have my own pool
	static void* operator new( size_t s,Runnable* owner )
	{
		return ::operator new (s);
	}

	static void operator delete( void* ptr )
	{
		::operator delete(ptr);
	}
	CustomProcessType()
	{
		//spdlog::debug("CustomProcessType constructor");
	}
	~CustomProcessType()
	{
		//spdlog::debug("CustomProcessType destructor");
	}
	private:
	int tema;
	/*
	no es necesario sobreescribirla
	virtual void onUpdate(uint64_t msecs) override
	{
		GenericProcess::onUpdate(msecs);
	}
	*/
};
//custom allocator for CustomProcessType
struct MyAllocator
{
	static CustomProcessType* allocate(Runnable* _this)
	{
		return new (_this)CustomProcessType();
	}
};
class MyProcess : public Process
{
	public:
		MyProcess(int& var):Process(),mVar(var){}
	private:
		void onUpdate(uint64_t) override
		{
			int aux = mVar;
			++mVar;
			text::debug("MyProcess");
			::tasking::Process::wait(70050);
			mVar = aux;
		}	
		int& mVar;

};
class MyTask
{
	public:
	MyTask(Process* target,int& var):mTarget(target),mVar(var){}
	::tasking::EGenericProcessResult operator()(uint64_t,Process*)
	{
		text::debug("MyTask");
		++mVar;
		if ( mTarget )
			mTarget->pause();
		tasking::Process::wait(213);
		if ( mTarget )
			mTarget->wakeUp();
		return ::tasking::EGenericProcessResult::CONTINUE;
	}
	private:
	Process* mTarget;
	int& mVar;
};
::tasking::EGenericProcessResult staticFuncTask(RUNNABLE_TASK_PARAMS,int& var)
{
	++var;
	text::debug("staticFuncTask");
	return ::tasking::EGenericProcessResult::CONTINUE;
}
static Timer sTimer;
/**
 * objetivo: muchas tareas concurrentes pero que al final un resultado no debe ser modificado, por lo que el test
 * se pasará si el valor sigue manteniendose
 */

uint64_t constexpr TIME_MARGIN = 10;
void CHECK_TIME(uint64_t t0, uint64_t t1, std::string text )
{
	auto elapsed = std::abs((int64_t)t1-(int64_t)t0);
	if ( elapsed > TIME_MARGIN)
		text::warn("Margin time overcome {}. Info: {}",elapsed,text);
}

static int _testMicroThreadingMonoThread(tests::BaseTest* test)
{
	#ifdef USE_SPDLOG
	spdlog::set_level(spdlog::level::debug);
	#endif
	using namespace std::string_literals;
	size_t s1 = sizeof(Process);
	size_t s2 = sizeof(GenericProcess);
	size_t s3 = sizeof(MThreadAttributtes);
	text::info("Process size {} ; GenericProcess size {}; MThreadAttributes {} ",s1,s2,s3);
	
	int result = 0;
	int sharedVar = 0;
	
	auto th1 = ThreadRunnable::create();
	/*th1->post([](RUNNABLE_TASK_PARAMS)
	{
		std::cout << "UNO" << std::endl;
		tasking::Process::wait(2500);
		std::cout << "DOS" << std::endl;
		return ::tasking::EGenericProcessResult::CONTINUE;
	},true,1000);
	Thread::sleep(120000);
	return 0;
*/
	auto th2 = ThreadRunnable::create(true);

	th1->post( [th2](RUNNABLE_TASK_PARAMS)
	{
		bool autokill = true;
		std::shared_ptr<Process> p1=th2->post([th2](uint64_t t,Process* p)
		{
			static bool firstTime = true;
			text::debug("Ejecuto p1");
			if ( firstTime )
			{
				firstTime = false;

				auto fut = th2->execute<int>(
					[]()
					{
						::tasking::Process::wait(20000);
						return 6;
					}
				);

		
				auto fr = ::tasking::waitForFutureMThread(fut,2000);
				if ( !fr.isValid())
					text::error(fr.error().errorMsg);
				else
					text::debug("{}",fr.value());
			
				// if ( fr != ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_OK)
				// {
				// 	spdlog::error(fut.getValue().error().errorMsg);
				// }
				text::debug("espero en p1");
				auto wr = tasking::Process::wait(10000);			
				text::debug("vuelvo a esperar en p1");

				// esto tengo que arreglarlo para que, si soy autokill, no vuelva a esperar si está en trying_tokill
				// meditar sobre estos temas, no me convence demasiado la forma en que está planteado
				//wr = tasking::Process::wait(10000);
				wr = tasking::Process::switchProcess(true);
				wr = tasking::Process::sleep();
				text::debug("fin espera en p1");
				
				//  no me convence nada el tema del kill y demas. Cosas que pasan:
				//   - aunque tenga el autokill a false, igual convenía enterarme mejor de que se me intenta matar. En realidad lo sé si consulto el state.Lo que

				//  - por otro lado, aunque tenga el autokill, después vuelve a hacerme el kill->tengo que impedirlo
				 
			
			}
			text::debug("Continuo");
			return ::tasking::EGenericProcessResult::CONTINUE;
		},autokill,2000,000);
		th2->fireAndForget(
			[p1]()
			{
				text::debug("Ejecuto p2");
				tasking::Process::wait(4000);
				// spdlog::debug("Pauso proceso");
				//  if ( p1 )
				//  	p1->pause();
			//	tasking::Process::wait(1000);
				text::debug("Mato proceso");
				p1->kill();
				// spdlog::debug("Vuelvo a pausar proceso");
				// if ( p1 )
				// 	p1->pause();
				tasking::Process::wait(25000);
				text::debug("Despierto proceso");
				if ( p1 )
				 	p1->wakeUp();
			}
		);

		text::debug("execution done");
		return ::tasking::EGenericProcessResult::KILL;
	}
	,true,3000);
	
	/*
	th1->post( [&sharedVar](RUNNABLE_TASK_PARAMS)
	{
		int aux = sharedVar;
		sharedVar++;
		spdlog::debug("Lambda");
		auto t1 = p->getElapsedTime();
		CHECK_TIME(t1,p->getPeriod(),"check period"s);
		auto t0 = sTimer.getMilliseconds();
		unsigned int waittime = 3550;
		::tasking::Process::wait(waittime);
		t1 = sTimer.getMilliseconds();
		CHECK_TIME(t1-t0,waittime,"check wait 1"s);
		waittime = 67;
		::tasking::Process::wait(waittime);
		auto t2 = sTimer.getMilliseconds();
		CHECK_TIME(t2-t1,waittime,"check wait 2"s);
		t0 = sTimer.getMilliseconds();
		::tasking::Process::wait(2000);
		t1 = sTimer.getMilliseconds();
		spdlog::debug("Elapsed {}",t1-t0);
		sharedVar = aux;
		return ::tasking::EGenericProcessResult::CONTINUE;
	},true,2000,000);
	*/
	// th1->post<CustomProcessType,MyAllocator>(
	// 	::mpl::linkFunctor<::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*)>(staticFuncTask,::mpl::_v1,::mpl::_v2,::mpl::_v3,mpl::createRef(sharedVar))
	// 	,true,4200);
	// auto p = make_shared<MyProcess>(sharedVar);
	// p->setPeriod(0);
	// th1->postTask(p);
	// th1->post(MyTask(p.get(),sharedVar),true,1200);

/*
preparar bien el test: quiero que los procesos actúa sobre algún objeto y tenga una salida precedible, por ejemplo:
 - incrementar/dec variable de forma que deba siempre ser isgreaterequal
- 
*/
	text::debug("finish");
	// th1->finish();
	// th1->join();
	
	return 0;
}
//check performance launching a lot of tasks
//@todo habrái que hacerlo con un profiler, un sistema de benchmarking...
int  _testPerformanceLotTasks(tests::BaseTest* test)
{
//algo tengo mal en los presets que en window dsrelease no tira, lo raro es que está gemnerando una caprta debug aunque sea relase
//parece ser que es por cosas del generador de MSVC que no sabe la configuracion
	int result = 0;
	constexpr int nIterations = 1;
	constexpr int nTasks = 100000;

	auto th1 =ThreadRunnable::create(true,nTasks); //GenericThread::createEmptyThread(true,true,nTasks);
	th1->start();	
	uint64_t t0,t1;
	int count = 0;
	t0 = sTimer.getMilliseconds();
	auto steps = nTasks/th1->getMaxPoolSize();
	
	for(int it = 0; it < nIterations; ++it)
	{
		for(int j = 0;j<steps;++j)
		{
			for(int i = 0; i < th1->getMaxPoolSize(); ++i)
			{
				++count;
				th1->post<CustomProcessType,MyAllocator>( [count](RUNNABLE_TASK_PARAMS)
				{
					return ::tasking::EGenericProcessResult::KILL;
				},false,1000,0);		
			}
			::Thread::sleep(1) ;//to wait for taks
		}
		Thread::sleep(10);
	}
	t1 = sTimer.getMilliseconds();	
	text::info("Time launching {} tasks with global new: {} msecs",nTasks,t1-t0);

	Thread::sleep(2000);
	t0 = sTimer.getMilliseconds();	;
	count = 0;
	for(int it = 0; it < nIterations; ++it)
	{
		for(int j = 0;j<steps;++j)
		{
			for(int i = 0; i < th1->getMaxPoolSize(); ++i)
			{
				++count;
				th1->post( [count](RUNNABLE_TASK_PARAMS)
				{
					//spdlog::debug("Lambda {}",count);
					return ::tasking::EGenericProcessResult::KILL;
				},false,1000,0);
			}	
			::Thread::sleep(1) ;//wait for tasks finished
		}
		Thread::sleep(10);
	}
	t1 = sTimer.getMilliseconds();
	text::info("Time launching {} tasks with default allocator: {} msecs",nTasks,t1-t0);
	//Thread::sleep(45000);	
	return result;
}
std::atomic<int> sCount(0);
int _test_concurrent_post( ::tests::BaseTest* test)
{	
	auto consumer =ThreadRunnable::create(true,500,2000);
	constexpr int NUM_POSTS = 4;
	constexpr int NUM_ITERS = 700;
	std::array< std::shared_ptr<ThreadRunnable>,60> producers;
	for(size_t i=0;i<producers.size();++i)
	{
		producers[i] = ThreadRunnable::create(true);
	}

	//consumer->suspend();

	for(int c = 0; c < NUM_ITERS; c++ )
	{
		for(size_t i=0;i<producers.size();++i)
		{
			producers[i]->fireAndForget(
				[consumer,NUM_POSTS]()
				{
					for(auto i = 0; i < NUM_POSTS; ++i)
					{
						//@todo así peta. Es algo de destruiccion del hilo, ya que esto hará que tarde mś que la espera a fin
						//if (::tasking::Process::wait(1000) == ::tasking::Process::ESwitchResult::ESWITCH_OK)
						{
							consumer->fireAndForget(
								[]()
								{
									++sCount;
								}
							);
						}
					}
				}
			);
		}
	}
	text::info("Esperando...");
	//consumer->resume();
	Thread::sleep(10000);
	if ( sCount == producers.size()*NUM_POSTS*NUM_ITERS )
		text::info("_test_concurrent_post OK. {} Posts",sCount.load());
	else
		text::error("_test_concurrent_post KO!! Some tasks were not executed. Posts: {} Count: {}",producers.size()*NUM_POSTS*NUM_ITERS,sCount.load());
	return 0;
}
int _throwExc(int& v)
{
	v = rand();
	throw v;
}
struct MyException
{
	int code;
	string msg;
};
void _throwMyException(int& v)
{
	v = rand();
//	throw v;
	throw MyException{v,"MyException "+std::to_string(v)};
}
int _testExceptions( tests::BaseTest* test)
{
	auto th1 = ThreadRunnable::create(true);
	th1->post(
		 [](RUNNABLE_TASK_PARAMS)
		 {
			int val;
		//	tasking::Process::wait(2000);
			try
			{
				text::info("Task1: Context switch");
				tasking::Process::wait(2000);
				text::info("Task1: Throw exception");
				//_throwExc(val);

				_throwMyException(val);
				text::error("Task1: After throw exception. Shouldn't occur");
			}catch(int v)
			{
				if ( v == val)
					text::info("Task1: Captured exception ok");
				else
					text::error("Task1:onExecuteAlltests Captured exception Invalid,. Thrown {}, Catched {}",val,v);
				tasking::Process::wait(5000);
			}catch(MyException& exc)
			{
				if ( exc.code == val)
				{
					text::info("Task1: Captured exception ok. msg ={}",exc.msg);
				}
				else
					text::error("Task1:onExecuteAlltests Captured exception Invalid,. Thrown {}, Catched {}",val,exc.code);
				tasking::Process::wait(5000);
			}			
			
			 return ::tasking::EGenericProcessResult::CONTINUE;
		 }
	);
	th1->post(
		 [](RUNNABLE_TASK_PARAMS)
		 {
			 int val;
			try
			{

				text::info("Task2: Context switch");
				tasking::Process::wait(3000);
				text::info("Task2: Throw exception");
				_throwExc(val);
			#if USE_SPDLOG
				spdlog::error("Task2: After throw exception. Shouldn't occur");
		#endif
			}catch( int v)
			{
				if ( v == val)
					text::info("Task2: Captured exception ok");
				else
					text::error("Task2: Captured exception Invalid,. Thrown {}, Catched {}",val,v);
			}			
			
			 return ::tasking::EGenericProcessResult::CONTINUE;
		 }
	);
	Thread::sleep(15000);
	return 0;
}


int TestThreading::onExecuteTest()
{
	int result = 1;
	typedef int(*TestType)(tests::BaseTest*);
	TestType defaultTest = ::test_threading::test_futures;
	auto opt = tests::CommandLine::getSingleton().getOption("n");
	if ( opt != nullopt)
	{
		try
		{
			auto n = std::stol(opt.value());
			switch(n)
			{
				case 0:
					result = _testMicroThreadingMonoThread( this );
					break;
				case 1:
					result = _testPerformanceLotTasks(this);
					break;
				case 2:
					result = ::test_threading::test_futures( this );
					break;
				case 3:
					result = _test_concurrent_post(this);
					break;
				case 4:
					result = _testExceptions(this);
					break;
				default:;					
			}
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}		
	}else
		result = defaultTest(this); //by default
		
	
	// th1->post( std::function<bool(uint64_t ,Process*)>([](RUNNABLE_TASK_PARAMS)
	// {
	// 	spdlog::info("Tas1");
	// 	return false;
	// }),true,2000);
	/*
	Future<int> result = th1->execute<int>(::std::function<int()>(
		[]() {
			return 15;
		})
		);
	result.wait();
	if (result.getValid())
		cout << result.getValue() << endl;
	else
		cout << result.getError()->errorMsg << endl;
		*/
	
	return 0;
}
void TestThreading::registerTest()
{
    TestManager::getSingleton().registerTest(TEST_NAME,"threading tests:\n - 0 = mono thread;\n - 1 = performance launching a bunch of tasks",make_unique<TestThreading>());
}
int TestThreading::onExecuteAllTests()
{
	_testMicroThreadingMonoThread( this );
	_testPerformanceLotTasks(this);
	::test_threading::test_futures(this);
	 _test_concurrent_post(this);	
	 _testExceptions(this);
	 return 0;
}