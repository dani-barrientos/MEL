#include <sstream>
#include <core/Exception.h>
#include <stdio.h>

using core::Exception;
using std::stringstream;

FOUNDATION_CORE_OBJECT_TYPEINFO_IMPL_ROOT(Exception)

Exception::Exception():
	mMessage(""),
	mCause(NULL) {	
}

Exception::Exception(const Exception* cause):
	mMessage(""),
	mCause(cause?cause->clone():NULL) {	
}

Exception::Exception(const char* msg,...):
	mMessage(msg),
	mCause(NULL) {
	FOUNDATION_CORE_EXCEPTION_VARARGS(msg,msg);
}

Exception::Exception(const char* msg,const Exception *cause,...):
	mMessage(msg),
	mCause(cause?cause->clone():NULL) {
	FOUNDATION_CORE_EXCEPTION_VARARGS(msg,cause);
}

Exception::~Exception() throw() {
	if (mCause) {
		delete mCause;
		mCause=NULL;
	}
}

Exception::Exception( const Exception& e )
{
	mMessage = e.mMessage;
	mDetailedMessageFinal = "";
	mMessageFinal = e.mMessageFinal;
	mCause = e.mCause?e.mCause->clone():NULL;
}

const Exception *Exception::getCause() const {
	return mCause;
}

const char* Exception::getMessage() const {
	if (!mMessageFinal.size()) {
		stringstream ss;
		ss << getMyType().getName() << ": " << mMessage;
		mMessageFinal=ss.str();
	}
	return mMessageFinal.c_str();
}

const char* Exception::getDetailedMessage() const {
	if (!mDetailedMessageFinal.size()) {
		stringstream ss;
		ss << getMyType().getName() << ": " << mMessage;
		if (mCause!=NULL)
			ss << "\n[Caused by: " << mCause->getDetailedMessage() << "]";
		mDetailedMessageFinal=ss.str();
	}
	return mDetailedMessageFinal.c_str();
}

void Exception::setMessage(const char* msg) {
	mMessage=msg;
}

void Exception::_onClone( Exception** exc ) const
{
	if ( !*exc ) {
		*exc = new Exception( mMessage.c_str() );
	}else {
		//already created,set attributes
		(*exc)->mMessage = mMessage;
		(*exc)->mCause=mCause?mCause->clone():NULL;
	}	
}