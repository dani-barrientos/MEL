#include <core/IllegalArgumentException.h>
using core::IllegalArgumentException;
#include <stdio.h>

FOUNDATION_CORE_OBJECT_TYPEINFO_IMPL(IllegalArgumentException,Exception)

IllegalArgumentException::IllegalArgumentException(const char* msg,...):
	Exception("") {
	FOUNDATION_CORE_EXCEPTION_VARARGS(msg,msg);
}

IllegalArgumentException::IllegalArgumentException(const char* msg,const Exception* cause,...):
	Exception("",cause) {
	FOUNDATION_CORE_EXCEPTION_VARARGS(msg,cause);
}


void IllegalArgumentException::_onClone( Exception** exc ) const
{
	if ( !*exc )
	{
		*exc = new IllegalArgumentException( getMessage() );
	}
	Exception::_onClone( exc );
}

IllegalArgumentException::IllegalArgumentException( const IllegalArgumentException& e ) : Exception( e )
{

}