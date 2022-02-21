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
#ifdef USE_SPDLOG
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>  // support for loading levels from the environment variable
#include <spdlog/fmt/ostr.h> // support for user defined types
#endif

#define LIST_OPTION "list"
#define TEST_OPTION "t"
#define ALL_TESTS_OPTION "a"
static void _initialize()
{
#ifdef USE_SPDLOG
	spdlog::info("Probando spdlog {}.{}.{}  !", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
	spdlog::error("Prueba error");
	spdlog::set_level(spdlog::level::err); // Set global log level to err
	spdlog::info("otra prueba");
	spdlog::error("Prueba error 2");
	spdlog::set_level(spdlog::level::debug); // Set global log level to debug
	#ifdef NDEBUG
	spdlog::info( "ESTAMOS EN RELEASE");
	#else
	spdlog::info( "ESTAMOS EN DEBUG");
	#endif
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
		#ifdef USE_SPDLOG
		spdlog::debug("\t{}\n",argv[i]);
		#endif
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
	if ( CommandLine::getSingleton().getOption(ALL_TESTS_OPTION))
	{		
		auto& testmap = TestManager::getSingleton().getTests();
		for(auto& test:testmap)
		{
			test.second.second->executeAllTests();
		}
		return 0;
	}else
	{
		tests::BaseTest* currTest(nullptr);
		auto testOpt = CommandLine::getSingleton().getOption(TEST_OPTION);
		if (  testOpt != std::nullopt )
		{
			const string& name = testOpt.value();
			currTest = TestManager::getSingleton().getTest(name).get();
			std::cout << "Running test: " << name << '\n';
			
		}
		// else
		// 	currTest = TestManager::getSingleton().getTest(test_threading::TEST_NAME);
		
		if (currTest)
			return currTest->executeTest();
		else return 0;	
	}
}

