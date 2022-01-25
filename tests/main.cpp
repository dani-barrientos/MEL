// sample.cpp : Defines the entry point for the application.
//
#include "test_callbacks/test.h"
#include "test_threading/test.h"
#include "test_parallelism/test.h"
#include <CommandLine.h>
using tests::CommandLine;
#include <TestManager.h>
using tests::TestManager;
#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>  // support for loading levels from the environment variable
#include <spdlog/fmt/ostr.h> // support for user defined types


#define LIST_OPTION "list"
#define TEST_OPTION "t"
/**
 * main tests execution.
 * command line:
 *  -list list all available test
 *  -t <NAME>  execute given test
 * eachs test has it's own command line arguments
 **/
#include <core/Future.h>
using core::Future;
struct Pepe
{
	Pepe(int aA,string aB):a(aA),b(std::move(aB)){}
	Pepe(){};
	Pepe( const Pepe& )
	{
		spdlog::debug("Constructor copia Pepe");
	}
	Pepe( Pepe&& )
	{
		spdlog::debug("Constructor move Pepe");
	}
	Pepe& operator=(const Pepe& o2){
		spdlog::debug("Asignacion copia Pepe");
		a = o2.a;b=o2.b;return *this;}
	Pepe& operator=(Pepe&& o2){
		spdlog::debug("Constructor move Pepe");
	a = o2.a;b=std::move(o2.b);return *this;}
	int a;string b;
};
int main(int argc, const char* argv[])
{	
//ver cómo furrula y 
	//tebngo que ver que tenga funcionalidad similar a los appedern, por el tema de redirigir la salida a donde quiera->creo que son los sink
	spdlog::info("Probando spdlog {}.{}.{}  !", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
	spdlog::error("Prueba error");
	spdlog::set_level(spdlog::level::err); // Set global log level to err
	spdlog::info("otra prueba");
	spdlog::error("Prueba error 2");
	spdlog::set_level(spdlog::level::debug); // Set global log level to debug

	Future<int> f;
	f.setValue(6);
	{
	Future<Pepe> f2;	 
	Pepe p{1,"dani"};
	f2.setValue(Pepe(1,"dani"));
	//f2.setValue(p);
	spdlog::debug(f2.getValue().isValid());
	f2.getValue().value();
	const auto& fv = f2.getValue();
	auto& v = fv.value();
	spdlog::debug(v.b);
	}
	Future<Pepe> f3;	 
	::core::ErrorInfo ei{5,"pepe"};
	//f3.setError(ei);
	f3.setError(::core::ErrorInfo(5,"pepe"));
	f3.getValue().error();

	Future<void> f4;
	f4.setValue();
	auto v = f4.getValue();
	spdlog::debug( v.isValid() );
	Future<void> f5;
	f5.setError(::core::ErrorInfo(5,"dani"));
	auto v2 = f5.getValue();
	spdlog::debug( v2.isValid() );
	
	#ifdef NDEBUG
	spdlog::info( "ESTAMOS EN RELEASE");
	#else
	spdlog::info( "ESTAMOS EN DEBUG");
	#endif
	
  	std::cout << "Running main with "<<argc<<" arguments:\n";
	for(int i =0;i<argc;++i)	
	{
		spdlog::debug("\t{}\n",argv[i]);
	}
	CommandLine::createSingleton(argc,argv);
	// const char* arg[] = {"kk","-list","-t","callbacks"};	
	// CommandLine::createSingleton(4,arg);
	test_callbacks::registerTest();
	test_threading::registerTest(); 
	test_parallelism::registerTest();

	if ( CommandLine::getSingleton().getOption(LIST_OPTION) != std::nullopt )
	{
		const auto& testmap = TestManager::getSingleton().getTests();
		for(const auto& [key,val]:testmap)
		{
			std::cout << key << " -> " << val.first <<'\n';
		}
	}
	TestManager::TestType currTest = nullptr;
	auto testOpt = CommandLine::getSingleton().getOption(TEST_OPTION);
	if (  testOpt != std::nullopt )
	{
		const string& name = testOpt.value();
		currTest = TestManager::getSingleton().getTest(name);
		std::cout << "Running test: " << name << '\n';
		
	}
	// else
	// 	currTest = TestManager::getSingleton().getTest(test_threading::TEST_NAME);
	
	if (currTest)
		return currTest();
	else return 0;
	
}
