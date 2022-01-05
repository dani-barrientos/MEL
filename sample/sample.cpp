// sample.cpp : Defines the entry point for the application.
//
#include "sample.h"
#include "test_callbacks/test.h"
#include "test_threading/test.h"

typedef int (*Test)();
int main(int argc, char* argv[])
{
 //@todo sistema genérico de test decente	
	Test currTest = test_callbacks::test;
	return currTest();
}
