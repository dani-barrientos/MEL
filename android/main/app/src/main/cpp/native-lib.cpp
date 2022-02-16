#include <jni.h>
#include <string>
#include <iostream>
#include "tests/test_callbacks/test.h"
#include "tests/test_threading/test.h"
#include "tests/test_parallelism/test.h"
#include "tests/test_execution/test.h"
#include <CommandLine.h>
using tests::CommandLine;
#include <TestManager.h>
using tests::TestManager;
#ifdef USE_SPDLOG
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>  // support for loading levels from the environment variable
#include <spdlog/fmt/ostr.h> // support for user defined types
#include "spdlog/sinks/android_sink.h"
#endif

extern "C" JNIEXPORT jstring JNICALL
Java_com_dabal_main_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "dabal tests";
    std::cout << hello; //@todo no vale con el NDK
    return env->NewStringUTF(hello.c_str());
}
#define TEST_OPTION "t"
extern "C" JNIEXPORT void JNICALL
Java_com_dabal_main_MainActivity_mainFromJNI(
        JNIEnv* env,
jobject /* this */) {
    //@todo por ahora de prueba. tengo que unificar este main
//sacar los parámetros de algún aldo
#ifdef USE_SPDLOG
    std::string tag = "spdlog-android";
    auto android_logger = spdlog::android_logger_mt("dabal", tag);
    //neede to change logger to be shown in logcat
    spdlog::set_default_logger(android_logger);
    android_logger->info("Use \"adb shell logcat\" to view this message.");
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
    const char* arg[] = {"","-t","execution","-n","1"};
    CommandLine::createSingleton(5,arg);
    test_callbacks::registerTest();
    test_threading::registerTest();
    test_parallelism::registerTest();
    test_execution::registerTest();
    TestManager::TestType currTest = nullptr;
    auto testOpt = CommandLine::getSingleton().getOption(TEST_OPTION);
    if (  testOpt != std::nullopt )
    {
        const string& name = testOpt.value();
        currTest = TestManager::getSingleton().getTest(name);
#ifdef USE_SPDLOG
        spdlog::info("Running test: {} ",name);
#endif
    }
    if (currTest)
        currTest();
}