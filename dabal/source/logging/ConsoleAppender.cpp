#include <logging/ConsoleAppender.h>
#include <iostream>
#include <stdio.h>
#include <core/CriticalSection.h>
#include <assert.h>
#ifdef _ANDROID
//note: remember to add -llog to LOCAL_LDLIBS in Android.mk!
#include <android/log.h>
#endif

#ifdef _IOS
#import <Foundation/Foundation.h>
#endif

using logging::ConsoleAppender;
using namespace logging;

::core::CriticalSection ConsoleAppender::CS;
//!@note: simple trick to avoid the "static initialization order fiasco"
core::CriticalSection& ConsoleAppender::CONSOLE_APPENDER_CS() {
	return CS;
}

#define CS_ENTER ConsoleAppender::CONSOLE_APPENDER_CS().enter();
#define CS_LEAVE ConsoleAppender::CONSOLE_APPENDER_CS().leave();

#ifdef _WINDOWS
static HANDLE SIMPLE_APPENDER_STDOUT=NULL;
static unsigned int SIMPLE_APPENDER_INSTANCES=0;
#define CONSOLE_BUFFER_WIDTH 160
#define CONSOLE_BUFFER_HEIGHT 4096

void initConsole() {
	CS_ENTER
		if (!SIMPLE_APPENDER_STDOUT) {
			if (!AllocConsole()) {
				assert(false && "Unable to allocate console!");
			}
			SetConsoleTitle("ConsoleAppender");
			SIMPLE_APPENDER_STDOUT=GetStdHandle(STD_OUTPUT_HANDLE);
			if (!SIMPLE_APPENDER_STDOUT) {
				assert(false && "Unable to retrieve console's stdout!");
			}
			else {
				COORD sz;
				sz.X=CONSOLE_BUFFER_WIDTH;
				sz.Y=CONSOLE_BUFFER_HEIGHT;
				SetConsoleScreenBufferSize(SIMPLE_APPENDER_STDOUT,sz);
			}
			HWND wnd=GetConsoleWindow();
			if (wnd) {
				HMENU menu=GetSystemMenu(wnd,FALSE);
				if (menu) {
					DeleteMenu(menu, SC_CLOSE, MF_BYCOMMAND);
				}
			}
		}
		++SIMPLE_APPENDER_INSTANCES;
		CS_LEAVE
}

void shutdownConsole() {
	CS_ENTER
		--SIMPLE_APPENDER_INSTANCES;
	if (SIMPLE_APPENDER_INSTANCES==0) {
		if (SIMPLE_APPENDER_STDOUT) {
			if (!FreeConsole()) {
				assert(false && "Unable to free console!");
			}
			SIMPLE_APPENDER_STDOUT=NULL;
		}
	}
	CS_LEAVE
}
WORD filter2att(const logging::LogFilter f) {
	switch (f) {
		case logging::LF_FATAL:
			//Bright red text / red background
			return FOREGROUND_RED | FOREGROUND_INTENSITY;
		case logging::LF_ERROR:
			//Red text / black background
			return FOREGROUND_RED;
		case logging::LF_WARN:
			//Yellow text /black background
			return FOREGROUND_RED | FOREGROUND_GREEN;
		case logging::LF_INFO:
			//Green text / black background
			return FOREGROUND_GREEN;
		case logging::LF_DEBUG:
		default:
			//White text / black background
			return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
	}
}
void consoleWrite(const string& txt,logging::LogFilter filter) {
	if (!SIMPLE_APPENDER_STDOUT)
		return;

	DWORD outChars;
	WORD textAtt=filter2att(filter);
	CS_ENTER
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo( SIMPLE_APPENDER_STDOUT, &csbi);
	SetConsoleTextAttribute(SIMPLE_APPENDER_STDOUT,textAtt);
	WriteConsole(SIMPLE_APPENDER_STDOUT,txt.c_str(),(DWORD)txt.size(),&outChars,NULL);	
	SetConsoleTextAttribute( SIMPLE_APPENDER_STDOUT, csbi.wAttributes );
	CS_LEAVE
}
#else
void initConsole() {}
void shutdownConsole() {}
void consoleWrite(const string& txt,logging::LogFilter filter) {
	CS_ENTER
#ifdef _ANDROID
	int prio;
	switch (filter) {
		case LF_DEBUG:prio=ANDROID_LOG_DEBUG;break;
		case LF_INFO:prio=ANDROID_LOG_INFO;break;
		case LF_WARN:prio=ANDROID_LOG_WARN;break;
		case LF_ERROR:prio=ANDROID_LOG_ERROR;break;
		case LF_FATAL:prio=ANDROID_LOG_FATAL;break;
		default: prio=ANDROID_LOG_DEFAULT;
	}
	__android_log_write(prio, "ConsoleAppender", txt.c_str());
#elif defined(_IOS)
    NSLog(@"%s",txt.c_str());
#else
	std::cout << txt.c_str();
#endif
	CS_LEAVE
}
#endif

ConsoleAppender::ConsoleAppender(Layout* layout):Appender(layout) {
	initConsole();
}

ConsoleAppender::~ConsoleAppender() {
	shutdownConsole();
}

void ConsoleAppender::deliver(const Message& msg) {
	//Build final output string and write it!
	string txt;
	getLayout()->format(msg,txt);
	consoleWrite(txt,msg.filter);
}

void ConsoleAppender::flush() {
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