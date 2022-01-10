#include "test.h"
#include <iostream>
#include <core/GenericThread.h>
using core::GenericThread;
using namespace std;
#include <TestManager.h>
using tests::TestManager;
#include <spdlog/spdlog.h>
/**
 * @todo pensar en test neceasrios:
 *  - mono hilo + microhilos
 *  - multihilo +`microhilos
 *  - futures y demas
 * 
 * @return int 
 */
static int test()
{
	auto th1 = GenericThread::createEmptyThread();
	th1->start();

	//@todo esto del sleep es una mierda. Tengo que diseñar bien para que no pasen estas cosas
	Thread::sleep(2000);  //@todo patraña, todo esto lo tengo mal estructurado
	th1->post( [](RUNNABLE_TASK_PARAMS)
	{
		spdlog::info("Task1");
		return ::core::EGenericProcessResult::KILL;
	},true,2000);
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
	Thread::sleep(30000);
	th1->finish();
	th1->join();
	return 0;
}
void test_threading::registerTest()
{
    TestManager::getSingleton().registerTest(TEST_NAME,"threading tests",test);
}
