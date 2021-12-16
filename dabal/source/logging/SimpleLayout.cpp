#include <sstream>
#include <ctime>
#include <logging/SimpleLayout.h>
#include <core/Timer.h>

using logging::SimpleLayout;
using logging::LogFilter;
using logging::Message;
using std::stringstream;

#include <stdio.h>
#define MAX_CUSTOM_ID_LENGTH 16
SimpleLayout::SimpleLayout(unsigned int aFlags, char aSeparator):
	mFlags(0) {
	setFlags(aFlags);
	setSeparator(aSeparator);
}

void SimpleLayout::setSeparator(char s) {
	mSeparator=s;
	mGlobalTimeFormat = "%Y-%m-%d %H:%M:%S";
	mGlobalTimeFormat += mSeparator;
	mRelativeTimeFormat = "%02d:%02d:%02d.%03d";
	mRelativeTimeFormat += mSeparator;
}

void SimpleLayout::setFlags(unsigned int f) {
	mFlags=f;
	mFixedSize=
		((mFlags&LF_GLOBALTIME)?21:0)+	//YYYY-MM-DD HH:MM::SS|
		((mFlags&LF_RELTIME)?13:0)+		//HH:MM:SS.MMM|
		((mFlags&LF_FILTER)?2:0)+		//F|
		((mFlags&LF_EXTRAINFO)?2:0)+	//\n\t
		((mFlags&LF_APPENDCR)?1:0)+		//\n
		4; //Just in case :P
}

void SimpleLayout::appendGlobalTime(string& str) {
	char tmp[32];
	time_t t=time(NULL);
	
	tm localTime;
	if (::core::Timer::localtime(&t, localTime))
	{
		strftime(tmp, 32, mGlobalTimeFormat.c_str(), &localTime);
		str.append(tmp);
	}
}

void SimpleLayout::appendRelativeTime(unsigned int millis,string& str) {
	char tmp[32];
	
	//Relative time (relative to app start)
	sprintf(tmp, mRelativeTimeFormat.c_str(),
			(int)(millis/(60000*24)%24),
			(int)(millis/60000)%60,
			(int)(millis/1000)%60,
			(int)(millis%1000));
	str.append(tmp);
}

void SimpleLayout::appendFilter(const Message& msg,string& str) {
	char tmp[MAX_CUSTOM_ID_LENGTH] = {0};	
	//tmp[1]=mSeparator;
	int separatorIndex = 1;
	switch (msg.filter) {
		case LF_DEBUG:
			tmp[0]='D';
			tmp[1]=mSeparator;
			break;
		case LF_INFO:
			tmp[0]='I';
			tmp[1]=mSeparator;
			break;
		case LF_WARN:
			tmp[0]='W';
			tmp[1]=mSeparator;
			break;
		case LF_ERROR:
			tmp[0]='E';
			tmp[1]=mSeparator;
			break;
		case LF_FATAL:
			tmp[0]='F';
			tmp[1]=mSeparator;
			break;
		case LF_CUSTOM:
			if(msg.customId[0] == 0)
			{
				tmp[0]='C';
				tmp[1]=mSeparator;
			}
			else
			{
				strncpy(tmp, msg.customId.c_str(), MAX_CUSTOM_ID_LENGTH-1);
				separatorIndex = msg.customId.length() > MAX_CUSTOM_ID_LENGTH-1 ? MAX_CUSTOM_ID_LENGTH-1 : (int)msg.customId.length();
			}
			break;
		default:
			tmp[0]='?';
	}
	tmp[separatorIndex]=mSeparator;
	str.append(tmp,separatorIndex+1);
}

void SimpleLayout::format(const Message& msg,string& result) {
	result.reserve(
				   mFixedSize +
				   (((mFlags&LF_FILTER) && (msg.filter&LF_CUSTOM))?msg.customId.length():0)+ 
				   ((mFlags&LF_TEXT)?msg.text.size():0)+
				   ((mFlags&LF_EXTRAINFO)?msg.extraInfo.size():0)
				   );
	
	if (mFlags & LF_GLOBALTIME) {
		appendGlobalTime(result);
	}
	
	if (mFlags & LF_RELTIME) {
		appendRelativeTime((unsigned int)msg.millis,result);
	}
	
	if (mFlags & LF_FILTER) {
		appendFilter(msg,result);
	}

	if (mFlags & LF_TEXT) {
		result.append(msg.text);
	}
	
	if ((mFlags & LF_EXTRAINFO) && msg.extraInfo.size()) {
		result.append("\n\t");
		result.append(msg.extraInfo);
	}
	
	if (mFlags & LF_APPENDCR)
		result.append("\n");
}
