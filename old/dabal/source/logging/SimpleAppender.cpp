#include <logging/SimpleAppender.h>
#include <iostream>
#include <stdio.h>
#ifdef _ANDROID
//note: remember to add -llog to LOCAL_LDLIBS in Android.mk!
#include <android/log.h>
#endif

#ifdef _IOS
#import <Foundation/Foundation.h>
#endif

using logging::SimpleAppender;
using namespace logging;

SimpleAppender::SimpleAppender(Layout* layout):Appender(layout) {
	//OK. Do nothing
}

#ifdef _WINDOWS
void simpleWrite(const string& txt,LogFilter f) {
	OutputDebugString(txt.c_str());
}
/*
#elif defined(_IOS) || defined(_MACOSX)
void simpleWrite(const string& txt) {	
	NSLog(@"%s",txt.c_str());
}
*/
#else
void simpleWrite(const string& txt,LogFilter f) {	
#ifdef _ANDROID
	int prio;
	switch (f) {
		case LF_DEBUG:prio=ANDROID_LOG_DEBUG;break;
		case LF_INFO:prio=ANDROID_LOG_INFO;break;
		case LF_WARN:prio=ANDROID_LOG_WARN;break;
		case LF_ERROR:prio=ANDROID_LOG_ERROR;break;
		case LF_FATAL:prio=ANDROID_LOG_FATAL;break;
		default: prio=ANDROID_LOG_DEFAULT;
	}
	__android_log_write(prio, "SimpleAppender", txt.c_str());
#elif defined(_IOS)
    NSLog(@"%s",txt.c_str());
#else
	std::cout << txt.c_str();
#endif
}
#endif

void SimpleAppender::deliver(const Message& msg) {
	//Build final output string and write it!
	string txt;
	getLayout()->format(msg,txt);
	simpleWrite(txt,msg.filter);
}

void SimpleAppender::flush() {
#ifdef _WINDOWS
	//OK. Nothing to do here
#else
	#ifdef _ANDROID
	//OK. Nothing to do here
	#elif defined(_IOS)
	//OK. Nothing to do here
	#else
		std::cout.flush();
	#endif
#endif
}