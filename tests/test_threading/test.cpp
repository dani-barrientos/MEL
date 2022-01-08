#include "test.h"
#include <iostream>
#include <core/GenericThread.h>
using core::GenericThread;
using namespace std;
#include <TestManager.h>
using tests::TestManager;
static int test()
{
cout << "Hello CMake" << endl;
	auto th1 = GenericThread::createEmptyThread();
	th1->start();

	//@todo esto del sleep es una mierda. Tengo que diseñar bien para que no pasen estas cosas
	//Thread::sleep(2000);  //@todo patraña, todo esto lo tengo mal estructuradp
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
	th1->finish();
	th1->join();
	return 0;
}
void test_threading::registerTest()
{
    TestManager::getSingleton().registerTest("threading","threading tests",test);
}