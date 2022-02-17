#include <jni.h>
#include <string>
#include <iostream>
#ifdef USE_SPDLOG
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>  // support for loading levels from the environment variable
#include <spdlog/fmt/ostr.h> // support for user defined types
#include "spdlog/sinks/android_sink.h"
#endif
#include "tests/tests_main.h"
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

//sacar los parámetros de algún aldo
#ifdef USE_SPDLOG
    std::string tag = "spdlog-android";
    auto android_logger = spdlog::android_logger_mt("dabal", tag);
    //neede to change logger to be shown in logcat
    spdlog::set_default_logger(android_logger);
#endif
#ifdef NDEBUG
#define loopSize "100000"
#else
#define loopSize "20000"
#endif
    /*
    //const char* arg[] = {"","-t","execution","-n","1","-ls",loopSize};
    //const char* arg[] = {"","-t","threading","-n","3"};
    const char* arg[] = {"","-t","callbacks"};
     */
    const char* arg[] = {"","-t","execution","-n","1","-ls",loopSize};
    int result = testsMain(7,arg);
}