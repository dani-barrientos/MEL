// sample.cpp : Defines the entry point for the application.
//
#include "sample.h"
#include <core/GenericThread.h>
using core::GenericThread;
using namespace std;

int main()
{
	cout << "Hello CMake." << endl;
	auto th1 = GenericThread::createEmptyThread();
	th1->start();

	//@todo esto del sleep es una mierda. Tengo que diseñar bien para que no pasen estas cosas
	Thread::sleep(2000);  //@todo patraña, todo esto lo tengo mal estructuradp
	Future<int> result = th1->execute<int>(::std::function<int()>(
		[]() {
			return 10;
		})
		);
	result.wait();
	if (result.getValid())
		std::cout << result.getValue();
	else
		std::cout << result.getError()->errorMsg;
	th1->finish();
	th1->join();
	return 0;
}
