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
/**
 * @todo pensar en test neceasrios:
 *  - mono hilo + microhilos
 *  - multihilo +`microhilos
 *  - futures y demas
 * 
 * @return int 
 */
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
			::core::Process::wait(70050);
			mVar = aux;
		}	
		int& mVar;

};
class MyTask
{
	public:
	MyTask(Process* target,int& var):mTarget(target),mVar(var){}
	::core::EGenericProcessResult operator()(uint64_t,Process*,::core::EGenericProcessState)
	{
		spdlog::debug("MyTask");
		++mVar;
		if ( mTarget )
			mTarget->pause();
		core::Process::wait(213);
		if ( mTarget )
			mTarget->wakeUp();
		return ::core::EGenericProcessResult::CONTINUE;
	}
	private:
	Process* mTarget;
	int& mVar;
};
::core::EGenericProcessResult staticFuncTask(RUNNABLE_TASK_PARAMS,int& var)
{
	++var;
	spdlog::debug("staticFuncTask");
	return ::core::EGenericProcessResult::CONTINUE;
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
static int _testMicroThreadingMonoThread()
{
	using namespace std::string_literals;
	size_t s1 = sizeof(Process);
	size_t s2 = sizeof(GenericProcess);
	size_t s3 = sizeof(MThreadAttributtes);
	spdlog::info("Process size {} ; GenericProcess size {}; MThreadAttributes {} ",s1,s2,s3);
	int result;
	int sharedVar = 0;
	auto th1 = GenericThread::createEmptyThread();
	th1->start();	
	//probar starttime
	th1->post( [&sharedVar](RUNNABLE_TASK_PARAMS)
	{
		int aux = sharedVar;
		sharedVar++;
		spdlog::debug("Lambda");
		auto t1 = p->getElapsedTime();
		CHECK_TIME(t1,p->getPeriod(),"check period"s);
		auto t0 = sTimer.getMilliseconds();
		unsigned int waittime = 3550;
		::core::Process::wait(waittime);
		t1 = sTimer.getMilliseconds();
		CHECK_TIME(t1-t0,waittime,"check wait 1"s);
		waittime = 67;
		::core::Process::wait(waittime);
		auto t2 = sTimer.getMilliseconds();
		CHECK_TIME(t2-t1,waittime,"check wait 2"s);
		sharedVar = aux;
		return ::core::EGenericProcessResult::CONTINUE;
	},true,2000,000);
	// th1->post(
	// 	::mpl::linkFunctor<::core::EGenericProcessResult,TYPELIST(uint64_t,Process*,::core::EGenericProcessState)>(staticFuncTask,::mpl::_v1,::mpl::_v2,::mpl::_v3,mpl::createRef(sharedVar))
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
	Thread::sleep(300000);
	th1->finish();
	th1->join();
	return result;
}
/**
 * @brief Tasking tests
 * commandline options
 * -n <number> -> test number:
 * 		0 = microthreading-mono thread
 * @return int 
 */
static int test()
{
	int result = 1;
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
				default:;					
			}
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}		
	}else
		result = _testMicroThreadingMonoThread(); //by default
	
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
    TestManager::getSingleton().registerTest(TEST_NAME,"threading tests",test);
}



