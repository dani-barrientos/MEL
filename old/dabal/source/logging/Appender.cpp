#include <logging/Appender.h>

using logging::Appender;
using logging::LogFilter;
using logging::Message;
using logging::Layout;

Appender::Appender(Layout* layout):mLayout(layout) {
}

void Appender::append(LogFilter filter,const unsigned int millis,const char* txt,const Exception*e,const char* customId) {
	if (mLayout) {
		Message msg;
		mLayout->build(filter,millis,txt,e,msg, customId);
		deliver(msg);
	}
}

Appender::~Appender() {
	if (mLayout) {
		delete mLayout;
		mLayout=NULL;
	}
}

void Appender::setLayout(Layout* l) {
	if (mLayout)
		delete mLayout;
	mLayout=l;
}

Layout* Appender::removeLayout() {
	Layout* r=mLayout;
	mLayout=NULL;
	return r;
}