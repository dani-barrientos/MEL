#include <core/UnsupportedOperationException.h>
using core::UnsupportedOperationException;
#include <stdio.h>

FOUNDATION_CORE_OBJECT_TYPEINFO_IMPL(UnsupportedOperationException,Exception)

UnsupportedOperationException::UnsupportedOperationException(const char* msg,...):
	Exception("") {
	FOUNDATION_CORE_EXCEPTION_VARARGS(msg,msg);
}

UnsupportedOperationException::UnsupportedOperationException(const char* msg,const Exception* cause,...):
	Exception("",cause) {
	FOUNDATION_CORE_EXCEPTION_VARARGS(msg,cause);
}

void UnsupportedOperationException::_onClone( Exception** exc ) const
{
	if (!*exc)
	{
		*exc = new UnsupportedOperationException( getMessage() );
	}
	Exception::_onClone( exc );
}

UnsupportedOperationException::UnsupportedOperationException( const UnsupportedOperationException& e ) : Exception( e )
{

}
