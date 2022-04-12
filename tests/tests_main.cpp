// sample.cpp : Defines the entry point for the application.
//
#include "tests_main.h"
#include "test_callbacks/test.h"
#include "test_threading/test.h"
#include "test_parallelism/test.h"
#include "test_execution/test.h"
#include <CommandLine.h>
using tests::CommandLine;
#include <TestManager.h>
using tests::TestManager;
#include <iostream>

#include <text/logger.h>

using namespace mel;
#define LIST_OPTION "list"
#define TEST_OPTION "t"
#define ALL_TESTS_OPTION "a"
static void _initialize()
{
	#ifdef USE_SPDLOG
	text::info("Using spdlog {}.{}.{}  !", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
	#endif

#ifdef NDEBUG
	text::info( "Release execution");
#else
	text::info( "Debug execution");
#endif
	test_callbacks::TestCallbacks::registerTest();
	test_threading::TestThreading::registerTest(); 
	test_parallelism::TestParallelism::registerTest();
	test_execution::TestExecution::registerTest();
}
int testsMain(int argc,const char* argv[])
{	
	_initialize();
	
  	std::cout << "Running main with "<<argc<<" arguments:\n";
	for(int i =0;i<argc;++i)	
	{
		text::info("\t{}\n",argv[i]);
	}
	CommandLine::createSingleton(argc,argv);
	// const char* arg[] = {"kk","-list","-t","callbacks"};	
	// CommandLine::createSingleton(4,arg);
	
	if ( CommandLine::getSingleton().getOption(LIST_OPTION) != std::nullopt )
	{
		const auto& testmap = TestManager::getSingleton().getTests();
		for(const auto& [key,val]:testmap)
		{
			std::cout << key << " -> " << val.first <<'\n';
		}
	}	
	auto allTestsOpt = CommandLine::getSingleton().getOption(ALL_TESTS_OPTION);
	auto testOpt = CommandLine::getSingleton().getOption(TEST_OPTION);
	if (  testOpt != std::nullopt )
	{
		tests::BaseTest* currTest(nullptr);
		const string& name = testOpt.value();
		currTest = TestManager::getSingleton().getTest(name).get();
		
		if (currTest)
		{			
			if ( allTestsOpt )
			{
				std::cout << "Running all tests for test: " << name << '\n';
				return currTest->executeAllTests();
			}
			else
			{
				std::cout << "Running test: " << name << '\n';
				return currTest->executeTest();
			}
		}
		else
		{
			std::cout << "Test " << name << " doesn't exist\n";
			return 0;
		}
	}else
	{
		if ( allTestsOpt)
		{		
			std::cout << "Running all test\n";
			auto& testmap = TestManager::getSingleton().getTests();
			for(auto& test:testmap)
			{
				test.second.second->executeAllTests();
			}
			return 0;
		}else
		{
			std::cout << "No tests to run!!\n";
		}
	}
	return 0;
}

