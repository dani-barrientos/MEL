#include "test.h"
#include <iostream>
#include <core/GenericThread.h>
using core::GenericThread;
using namespace std;
#include <TestManager.h>
using tests::TestManager;
#include <spdlog/spdlog.h>
#include <CommandLine.h>
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
	private:
	void onUpdate(uint64_t) override
	{
		spdlog::debug("MyProcess");
		::core::Process::wait(750);
	}	
};
class MyTask
{
	public:
	::core::EGenericProcessResult operator()(uint64_t,Process*,::core::EGenericProcessState)
	{
		spdlog::debug("MyTask");
		core::Process::wait(213);
		return ::core::EGenericProcessResult::CONTINUE;
	}
};
::core::EGenericProcessResult staticFuncTask(RUNNABLE_TASK_PARAMS)
{
	spdlog::debug("staticFuncTask");
	return ::core::EGenericProcessResult::CONTINUE;
}
/**
 * objetivo: muchas tareas concurrentes pero que al final un resultado no debe ser modificado, por lo que el test
 * se pasará si el valor sigue manteniendose
 */
static int _testMicroThreadingMonoThread()
{
	int result;
	auto th1 = GenericThread::createEmptyThread();
	th1->start();
//@todo esto del sleep es una mierda. Tengo que diseñar bien para que no pasen estas cosas
	//Thread::sleep(2000);  //@todo patraña, todo esto lo tengo mal estructurado, si no no se pueden postear cosas todavia
	th1->post( [](RUNNABLE_TASK_PARAMS)
	{
		spdlog::debug("Task1");
		::core::Process::wait(1550);
		return ::core::EGenericProcessResult::CONTINUE;
	},true,2000);
	th1->post(MyTask(),true,1500);
	th1->post(staticFuncTask,true,1200);
	auto p = make_shared<MyProcess>();
	p->setPeriod(2500);
	th1->postTask(p);

	Thread::sleep(30000);
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



