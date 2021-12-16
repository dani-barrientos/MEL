#include <core/ProcessException.h>
using core::ProcessException;

ProcessException::ProcessException(const char *msg,...):Exception("")
{
	FOUNDATION_CORE_EXCEPTION_VARARGS(msg,msg);
}
ProcessException::ProcessException( const ProcessException& e ):Exception( e ){}
void ProcessException::throwSame() const 
{
	throw ProcessException(*this);
}

void ProcessException::_onClone(Exception** exc ) const 
{
	if ( !*exc ) {
		*exc = new ProcessException("");
	}
	Exception::_onClone( exc );
}