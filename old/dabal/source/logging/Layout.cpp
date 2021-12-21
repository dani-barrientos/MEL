#include <logging/Layout.h>

using logging::Layout;
using logging::Message;

Layout::~Layout() {
	//OK. Do nothing
}

void Layout::build(const logging::LogFilter filter,const unsigned int time,const char* msg,const Exception* cause,Message& result, const char* customId) {
	result.filter=filter;
	result.millis=time;
	result.text=msg?msg:"";
	result.customId=customId?customId:"";
	result.extraInfo=cause?cause->getDetailedMessage():"";	
}
