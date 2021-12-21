#include <stdarg.h>
#include <logging/Logger.h>
#include <logging/SimpleAppender.h>
#include <logging/SimpleLayout.h>

using logging::Logger;
using logging::SimpleLayout;
using logging::SimpleAppender;
using logging::Appender;
using logging::LogFilter;
#include <stdlib.h>
#include <stdio.h>

using namespace logging;

#if defined(NDEBUG)
Logger ROOT_LOGGER_DEFAULT(logging::LF_WARN | logging::LF_ERROR | logging::LF_FATAL,new SimpleAppender(new SimpleLayout()));
#else
Logger ROOT_LOGGER_DEFAULT(logging::LF_INFO | logging::LF_WARN | logging::LF_ERROR | logging::LF_FATAL,new SimpleAppender(new SimpleLayout()));
#endif

Logger* Logger::mROOT_LOGGER=&ROOT_LOGGER_DEFAULT;
CriticalSection Logger::mCS;
LoggerMap Logger::mLOGGERS;
bool Logger::mINITIALIZED=false;

void Logger::init() {
	mCS.enter();
	if (mINITIALIZED)
		throw Exception("Logger already initialized!");
	mINITIALIZED=true;
	mCS.leave();
}

Logger::Logger(LogFilters filters,Appender* appender):
	mFilters(filters),
	mTimer(new Timer()),
	mAppenders() {
	if (appender!=NULL)
		mAppenders.push_back(appender);
}

Logger::~Logger() {
	clearAppenders();
}

void Logger::clearAppenders() {
	vector<Appender*>::iterator i=mAppenders.begin();
	while (i!=mAppenders.end()) {
		delete *i;
		++i;
	}
	mAppenders.clear();
}

void Logger::attach(Appender* appender) {
	if (appender!=NULL)
		mAppenders.push_back(appender);
}

void Logger::detach(Appender* appender) {
	vector<Appender*>::iterator i=mAppenders.begin();
	Appender* a;
	while (i!=mAppenders.end()) {
		if ((a=*i)==appender) {
			mAppenders.erase(i);
			return;
		}
		i++;
	}
}

Appender* Logger::detach(unsigned int idx) {
	Appender* a=mAppenders[idx];
	mAppenders.erase(mAppenders.begin()+idx);
	return a;
}

void Logger::append(LogFilter filter,const char* msg,const Exception *e, const char* customId) {
	vector<Appender*>::const_iterator i=mAppenders.begin();
	Appender* a;
	while (i!=mAppenders.end()) {
		a=*i++;
		a->append(filter,mTimer==(Timer*)NULL?0:(unsigned int)mTimer->getMilliseconds(),msg,e, customId);
	}
}

#define MAX_DYNAMIC_STRING_LENGTH 1024
#define VARARGS_START(nArg) \
	va_list ap; \
	va_start(ap,nArg); \
	size_t maxLng=strlen(msg)+MAX_DYNAMIC_STRING_LENGTH;\
	char* message= (char*)malloc(maxLng);\
	vsnprintf(message,1024,msg, ap);\
	message[MAX_DYNAMIC_STRING_LENGTH-1]=0;\
	va_end(ap);
#define VARARGS_END \
	free( message );

void Logger::debugf(const char* msg, uint32_t nArgs, ...) {

	if (isDebug()) {
		if (nArgs) {
			VARARGS_START(nArgs);
			append(LF_DEBUG, message, NULL);
			VARARGS_END;
		}
		else {
			append(LF_DEBUG, msg, nullptr);
		}
	}
}
void Logger::debug(const char* msg) {
	if (isDebug())
		append(LF_DEBUG, msg, nullptr);
}

void Logger::debugf(const char* msg,const Exception *e, uint32_t nArgs, ...) {
	if (isDebug()) {
		if (nArgs) {
			VARARGS_START(nArgs);
			append(LF_DEBUG, message, e);
			VARARGS_END;
		}
		else {
			append(LF_DEBUG, msg, e);
		}
	}
}
void Logger::debug(const char* msg, const Exception *e) {
	if (isDebug())
		append(LF_DEBUG, msg, e);
}

void Logger::vdebug(const char* msg, uint32_t nArgs, va_list arglist) {
	if (isDebug()) {
		if (nArgs) {
			size_t maxLng = strlen(msg) + MAX_DYNAMIC_STRING_LENGTH;
			char* message = (char*)malloc(maxLng);
			vsnprintf(message, 1024, msg, arglist);
			message[MAX_DYNAMIC_STRING_LENGTH - 1] = 0;
			append(LF_DEBUG, message, nullptr);
			free(message);
		}
		else {
			append(LF_DEBUG, msg, nullptr);
		}
	}
}

void Logger::infof(const char* msg, uint32_t nArgs, ...) {
	if (isInfo()) {
		if (nArgs) {
			VARARGS_START(nArgs);
			append(LF_INFO, message, nullptr);
			VARARGS_END;
		}
		else {
			append(LF_INFO, msg, nullptr);
		}
	}
}

void Logger::info(const char* msg) {
	if (isInfo())
		append(LF_INFO, msg, nullptr);
}

void Logger::infof(const char* msg,const Exception *e, uint32_t nArgs, ...) {
	if (isInfo()) {
		if (nArgs) {
			VARARGS_START(nArgs);
			append(LF_INFO, message, e);
			VARARGS_END;
		}
		else {
			append(LF_INFO, msg, e);
		}
	}
}
void Logger::info(const char* msg, const Exception *e) {
	if (isInfo()) {
		append(LF_INFO, msg, e);
	}
}

void Logger::vinfo(const char* msg, uint32_t nArgs, va_list arglist) {
	if (isInfo()) {
		if (nArgs) {
			size_t maxLng = strlen(msg) + MAX_DYNAMIC_STRING_LENGTH;
			char* message = (char*)malloc(maxLng);
			vsnprintf(message, 1024, msg, arglist);
			message[MAX_DYNAMIC_STRING_LENGTH - 1] = 0;
			append(LF_INFO, message, nullptr);
			free(message);
		}
		else {
			append(LF_INFO, msg, nullptr);
		}
	}
}

void Logger::warnf(const char* msg, uint32_t nArgs, ...) {
	if (isWarn()) {
		if (nArgs) {
			VARARGS_START(nArgs);
			append(LF_WARN, message, nullptr);
			VARARGS_END;
		}
		else {
			append(LF_WARN, msg, nullptr);
		}
	}
}
void Logger::warn(const char* msg) {
	if (isWarn()) {
		append(LF_WARN, msg, nullptr);
	}
}
void Logger::warnf(const char* msg,const Exception *e, uint32_t nArgs, ...) {	
	if (isWarn()) {
		if (nArgs) {
			VARARGS_START(nArgs);
			append(LF_WARN, message, e);
			VARARGS_END;
		}
		else {
			append(LF_WARN, msg, e);
		}
	}
}
void Logger::warn(const char* msg, const Exception *e) {
	if (isWarn()) {
		append(LF_WARN, msg, e);
	}
}

void Logger::vwarn(const char* msg, uint32_t nArgs, va_list arglist) {
	if (isWarn()) {
		if (nArgs) {
			size_t maxLng = strlen(msg) + MAX_DYNAMIC_STRING_LENGTH;
			char* message = (char*)malloc(maxLng);
			vsnprintf(message, 1024, msg, arglist);
			message[MAX_DYNAMIC_STRING_LENGTH - 1] = 0;
			append(LF_WARN, message, nullptr);
			free(message);
		}
		else {
			append(LF_WARN, msg, nullptr);
		}
	}
}

void Logger::errorf(const char* msg, uint32_t nArgs, ...) {
	if (isError()) {
		if (nArgs) {
			VARARGS_START(nArgs);
			append(LF_ERROR, message, nullptr);
			VARARGS_END;
		}
		else {
			append(LF_ERROR, msg, nullptr);
		}
	}
}
void Logger::error(const char* msg) {
	if (isError()) {
		append(LF_ERROR, msg, nullptr);
	}
}

void Logger::errorf(const char* msg, const Exception *e, uint32_t nArgs, ...) {
	if (isError()) {
		if (nArgs) {
			VARARGS_START(nArgs);
			append(LF_ERROR, message, e);
			VARARGS_END;
		}
		else {
			append(LF_ERROR, msg, e);
		}
	}
}
void Logger::error(const char* msg, const Exception *e) {
	if (isError()) {
		append(LF_ERROR, msg, e);
	}
}

void Logger::verror(const char* msg, uint32_t nArgs, va_list arglist) {
	if (isError()) {
		if (nArgs) {
			size_t maxLng = strlen(msg) + MAX_DYNAMIC_STRING_LENGTH;
			char* message = (char*)malloc(maxLng);
			vsnprintf(message, 1024, msg, arglist);
			message[MAX_DYNAMIC_STRING_LENGTH - 1] = 0;
			append(LF_ERROR, message, nullptr);
			free(message);
		}
		else {
			append(LF_ERROR, msg, nullptr);
		}
	}
}

void Logger::fatalf(const char* msg, uint32_t nArgs, ...) {
	if (isFatal()) {
		if (nArgs) {
			VARARGS_START(nArgs);
			append(LF_FATAL, message, nullptr);
			VARARGS_END;
		}
		else {
			append(LF_FATAL, msg, nullptr);
		}
	}
}
void Logger::fatal(const char* msg) {
	if (isFatal()) {
		append(LF_FATAL, msg, nullptr);
	}
}

void Logger::fatalf(const char* msg,const Exception *e, uint32_t nArgs, ...) {
	if (isFatal()) {
		if (nArgs) {
			VARARGS_START(nArgs);
			append(LF_FATAL, message, e);
			VARARGS_END;
		}
		else {
			append(LF_FATAL, msg, e);
		}
	}
}
void Logger::fatal(const char* msg, const Exception *e) {
	if (isFatal()) {
		append(LF_FATAL, msg, e);
	}
}

void Logger::vfatal(const char* msg, uint32_t nArgs, va_list arglist) {
	if (isFatal()) {
		if (nArgs) {
			size_t maxLng = strlen(msg) + MAX_DYNAMIC_STRING_LENGTH;
			char* message = (char*)malloc(maxLng);
			vsnprintf(message, 1024, msg, arglist);
			message[MAX_DYNAMIC_STRING_LENGTH - 1] = 0;
			append(LF_FATAL, message, nullptr);
			free(message);
		}
		else {
			append(LF_FATAL, msg, nullptr);
		}
	}
}

void Logger::customf(const char* id, const char* msg, uint32_t nArgs, ...)
{
	if (isCustom()) {
		if (nArgs) {
			VARARGS_START(nArgs);
			append(LF_CUSTOM, message, nullptr, id);
			VARARGS_END;
		}
		else {
			append(LF_CUSTOM, msg, nullptr, id);
		}
	}
}
void Logger::custom(const char* id, const char* msg)
{
	if (isCustom()) {
		append(LF_CUSTOM, msg, nullptr, id);
	}
}

void Logger::customf(const char* id, const char* msg,const Exception *e, uint32_t nArgs, ...)
{
	if (isCustom()) {
		if (nArgs) {
			VARARGS_START(nArgs);
			append(LF_CUSTOM, message, e, id);
			VARARGS_END;
		}
		else {
			append(LF_CUSTOM, msg, e, id);
		}
	}
}
void Logger::custom(const char* id, const char* msg, const Exception *e) {
	if (isCustom()) {
		append(LF_CUSTOM, msg, e, id);
	}
}

void Logger::vcustom(const char* msg, uint32_t nArgs, va_list arglist) {
	if (isCustom()) {
		if (nArgs) {
			size_t maxLng = strlen(msg) + MAX_DYNAMIC_STRING_LENGTH;
			char* message = (char*)malloc(maxLng);
			vsnprintf(message, 1024, msg, arglist);
			message[MAX_DYNAMIC_STRING_LENGTH - 1] = 0;
			append(LF_CUSTOM, message, nullptr);
			free(message);
		}
		else {
			append(LF_CUSTOM, msg, nullptr);
		}
	}
}

void Logger::setTimer(SmartPtr<Timer> timer) {
	mTimer = timer;
}

void Logger::shutdown() {
	LoggerMap::iterator i=mLOGGERS.begin();
	//Delete all loggers (but the default one)
	for (;i!=mLOGGERS.end();++i) {
		if (i->second!=&ROOT_LOGGER_DEFAULT)
			delete i->second;
	}
	mLOGGERS.clear();

	//Delete the root logger (but the default one)
	if (mROOT_LOGGER!=&ROOT_LOGGER_DEFAULT) {
		delete mROOT_LOGGER;
		mROOT_LOGGER=NULL;
	}
	mINITIALIZED=false;
}

void Logger::setRootLogger(Logger* l) {
	mCS.enter();
	//Only delete the root logger if it's not the default one (as it's a
	//static object)
	if (mROOT_LOGGER && mROOT_LOGGER!=&ROOT_LOGGER_DEFAULT)
		delete mROOT_LOGGER;
	mROOT_LOGGER=l;
	mCS.leave();
}

void Logger::flush() {
	std::vector<Appender*>::iterator i=mAppenders.begin();
	for (;i!=mAppenders.end();++i) {
		(*i)->flush();
	}
}