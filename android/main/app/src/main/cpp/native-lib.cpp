#include <jni.h>
#include <string>
#include <iostream>
#include <cstring>
#ifdef USE_SPDLOG
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>  // support for loading levels from the environment variable
#include <spdlog/fmt/ostr.h> // support for user defined types
#include "spdlog/sinks/android_sink.h"
#endif
#include "tests/tests_main.h"
extern "C" JNIEXPORT jstring JNICALL
Java_com_mel_main_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */,
        jstring commandLine) {
    std::string hello = "mel tests";
    std::cout << hello; //@todo no vale con el NDK
    return env->NewStringUTF(hello.c_str());
}
#define TEST_OPTION "t"
extern "C" JNIEXPORT void JNICALL
Java_com_mel_main_MainActivity_mainJNI(
        JNIEnv* env,
jobject /* this */,jobjectArray commands) {

#ifdef USE_SPDLOG
    std::string tag = "spdlog-android";
    auto android_logger = spdlog::android_logger_mt("mel", tag);
    //need to change logger to be shown in logcat
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
    size_t size = env->GetArrayLength(commands);
    const char** args;
    args = new const char*[size+1];
    args[0] = ""; //tests need empty first arg
    for(size_t i = 0; i < size; ++i)
    {
        jstring str = (jstring)env->GetObjectArrayElement(commands,i);
        const char* rawStr = env->GetStringUTFChars(str,JNI_FALSE);
        char* aux = new char[strlen(rawStr)];
        std::strcpy(aux,rawStr);
        args[i+1] = aux;
        env->ReleaseStringUTFChars(str,rawStr);
    }
    //const char* arg[] = {"","-t","execution","-n","1","-ls",loopSize};
    int result = testsMain(size + 1,args);
    //int result = testsMain(size + 1,arg);
    for(size_t i = 1; i < size+1; ++i)
    {
        delete [] args[i];
    }

}
extern "C" JNIEXPORT void JNICALL
Java_com_mel_main_MainActivity_allTestsJNI(
        JNIEnv* env,
        jobject /* this */)
{
#ifdef USE_SPDLOG
    std::string tag = "spdlog-android";
    auto android_logger = spdlog::android_logger_mt("mel", tag);
    //need to change logger to be shown in logcat
    spdlog::set_default_logger(android_logger);
#endif
    const char* arg[] = {"","-t","a"};
    int result = testsMain(3,arg);
}