#include "test.h"
#include <iostream>
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
#include "future_tests.h"
#include <tasking/utilities.h>

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
			spdlog::debug("MyProcess");
			::tasking::Process::wait(70050);
			mVar = aux;
		}	
		int& mVar;

};
class MyTask
{
	public:
	MyTask(Process* target,int& var):mTarget(target),mVar(var){}
	::tasking::EGenericProcessResult operator()(uint64_t,Process*,::tasking::EGenericProcessState)
	{
		spdlog::debug("MyTask");
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
	spdlog::debug("staticFuncTask");
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
		spdlog::warn("Margin time overcome {}. Info: {}",elapsed,text);
}
//pruebas---
class Continuation;
//sería clase genérica que usa cualquier agente de ejecución->probar uno con threadPool y que distribuya tareas
class Executor
{
	friend class ContinuationData;
	public:
		Executor(Runnable* runnable):mRunnable(runnable)
		{

		};
		template <class F> Continuation launch( F&& f);
	private:
		Runnable* mRunnable;
		template <class F> void _execute(F&& f)
		{
			mRunnable->fireAndForget(std::forward<F>(f));
		}
};

class ContinuationData final : public enable_shared_from_this<ContinuationData>
{
	friend class Continuation;
	public:
	//	static std::atomic<int> counter; //para pruebas
		template <class F> ContinuationData(Executor ex,F&& f ):mState(EState::NONE), mExecutor(std::move(ex)),mFunct(std::forward<F>(f)),mNext(nullptr)
		{			
	//		++counter;
		}
		~ContinuationData()
		{
	//		spdlog::debug("~ContinuationData()");
	//		--counter;
		}
		template <class F> Continuation next(F&& f);		
		void start()
		{
			mExecutor._execute(
				[_this = shared_from_this()]() mutable
				{					
					::core::Lock lck(_this->mCS);				
					_this->mFunct();
					_this->mState = EState::DONE;
					if ( _this->mNext )
						_this->mNext->start();
				}
			);
		}
	private:	
		enum class EState :uint8_t{NONE,DOING,DONE};
		CriticalSection mCS;
		Executor	mExecutor;
		EState mState;		
		std::shared_ptr<ContinuationData> mNext;
		std::function<void()> mFunct;
	
};
//std::atomic<int> ContinuationData::counter = 0;
class Continuation
{
	friend class Executor;
	
	public:
		template <class F> Continuation(Executor ex,F&& f ):mData(make_shared<ContinuationData>(ex,std::forward<F>(f))){}		
		template <class F> Continuation next(F&& f)
		{
			return mData->next(std::forward<F>(f));
		}
		Continuation(shared_ptr<ContinuationData> data):mData(data){}  //@todo debe ser privado pero algo no me va bien con el friend
		Continuation(Continuation&& cont):mData(std::move(cont.mData)){}
		Continuation(const Continuation& cont):mData(cont.mData){}
	private:
		void _start(){ if(mData) mData->start();}
		shared_ptr<ContinuationData> mData;

};
template <class F> Continuation ContinuationData::next(F&& f)
{
	::core::Lock lck(mCS);
	if ( mState== EState::DONE)
	{
		//spdlog::debug("next. Straight execution");
		//straight execution 
		return mExecutor.launch(f);
	}else
	{
		//spdlog::debug("next. Not available");
		mNext = make_shared<ContinuationData>(mExecutor,std::forward<F>(f));
		return Continuation(mNext); 
	}
}	
template <class F> Continuation Executor::launch( F&& f )
{
	Continuation result(*this,std::forward<F>(f));
	result._start();
	return result;
}	


static int _testMicroThreadingMonoThread()
{
	
	using namespace std::string_literals;
	size_t s1 = sizeof(Process);
	size_t s2 = sizeof(GenericProcess);
	size_t s3 = sizeof(MThreadAttributtes);
	spdlog::info("Process size {} ; GenericProcess size {}; MThreadAttributes {} ",s1,s2,s3);
	int result = 0;
	int sharedVar = 0;
	auto th1 = GenericThread::createEmptyThread(false);
	//th1->start();	
	{
//---- pruebas executors
	Executor ex(th1.get());
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
//	spdlog::debug("Remaining Data 3: {}",ContinuationData::counter);	
	th1->start();	
	Thread::sleep(2000);
//	spdlog::debug("Final Remaining Data: {}",ContinuationData::counter);
//---
	/*
	auto th2 = GenericThread::createEmptyThread();
	th1->post( [th2](RUNNABLE_TASK_PARAMS)
	{
		th2->fireAndForget([]
		{
			spdlog::debug("HECHO!!!");
		},5000);

//revisar execute

		 auto r = th2->execute<int>(
			[]()
			{
				::Process::wait(3000);
				return 6;
			}
		);
		spdlog::debug("waiting for execution");
		tasking::waitForFutureMThread(r);
		spdlog::debug("execution done");
		return ::tasking::EGenericProcessResult::CONTINUE;
	},true,3000);
	*/
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
	// 	::mpl::linkFunctor<::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*,::tasking::EGenericProcessState)>(staticFuncTask,::mpl::_v1,::mpl::_v2,::mpl::_v3,mpl::createRef(sharedVar))
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
	Thread::sleep(60000);
	spdlog::debug("finish");
	th1->finish();
	th1->join();
	return result;
}
//check performance launching a lot of tasks
//@todo habrái que hacerlo con un profiler, un sistema de benchmarking...
int  _testPerformanceLotTasks()
{
//algo tengo mal en los presets que en window dsrelease no tira, lo raro es que está gemnerando una caprta debug aunque sea relase
//parece ser que es por cosas del generador de MSVC que no sabe la configuracion
	int result = 0;
	constexpr int nIterations = 1;
	constexpr int nTasks = 100000;

	auto th1 = GenericThread::createEmptyThread(true,true,nTasks);
	th1->start();	
	uint64_t t0,t1;
	int count = 0;
	t0 = sTimer.getMilliseconds();
	auto steps = nTasks/th1->getMaxPoolTasks();
	
	for(int it = 0; it < nIterations; ++it)
	{
		for(int j = 0;j<steps;++j)
		{
			for(int i = 0; i < th1->getMaxPoolTasks(); ++i)
			{
				++count;
				th1->post<CustomProcessType,MyAllocator>( [count](RUNNABLE_TASK_PARAMS)
				{
					//spdlog::debug("Lambda {}",count);
					return ::tasking::EGenericProcessResult::KILL;
				},true,1000,0);		
			}
			::Thread::sleep(1) ;//to wait for taks
		}
		Thread::sleep(10);
	}
	t1 = sTimer.getMilliseconds();	
	spdlog::info("Time launching {} tasks with global new: {} msecs",nTasks,t1-t0);
	Thread::sleep(2000);
	t0 = sTimer.getMilliseconds();	;
	count = 0;
	for(int it = 0; it < nIterations; ++it)
	{
		for(int j = 0;j<steps;++j)
		{
			for(int i = 0; i < th1->getMaxPoolTasks(); ++i)
			{
				++count;
				th1->post( [count](RUNNABLE_TASK_PARAMS)
				{
					//spdlog::debug("Lambda {}",count);
					return ::tasking::EGenericProcessResult::KILL;
				},true,1000,0);
			}	
			::Thread::sleep(1) ;//wait for tasks finished
		}
		Thread::sleep(10);
	}
	t1 = sTimer.getMilliseconds();
	spdlog::info("Time launching {} tasks with default allocator: {} msecs",nTasks,t1-t0);
	Thread::sleep(15000);
	th1->finish();
	th1->join();
	return result;
}


/**
 * @brief Tasking tests
 * commandline options
 * -n <number> -> test number:
 * 		0 = microthreading-mono thread
 * 		1 = lots of tasks
 * 		2 = Future uses
 * @return int 
 */
static int test()
{
	int result = 1;
	TestManager::TestType defaultTest = _testPerformanceLotTasks;
	auto opt = tests::CommandLine::getSingleton().getOption("n");
	if ( opt != nullopt)
	{
		try
		{
			auto n = std::stol(opt.value());
			switch(n)
			{
				case 0:
					result = _testMicroThreadingMonoThread();
					break;
				case 1:
					result = _testPerformanceLotTasks();
					break;
				case 2:
					result = ::test_threading::test_futures();
					break;
				default:;					
			}
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}		
	}else
		result = defaultTest(); //by default
		
	
	// th1->post( std::function<bool(uint64_t ,Process*,::core::EGenericProcessState)>([](RUNNABLE_TASK_PARAMS)
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
void test_threading::registerTest()
{
    TestManager::getSingleton().registerTest(TEST_NAME,"threading tests:\n - 0 = mono thread;\n - 1 = performance launching a bunch of tasks",test);
}
