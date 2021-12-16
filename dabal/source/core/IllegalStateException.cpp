#include <core/IllegalStateException.h>
using core::IllegalStateException;
#include <stdio.h>

FOUNDATION_CORE_OBJECT_TYPEINFO_IMPL(IllegalStateException,Exception)

IllegalStateException::IllegalStateException(const char* msg,...):Exception("") {
	FOUNDATION_CORE_EXCEPTION_VARARGS(msg,msg);
}

IllegalStateException::IllegalStateException(const char* msg,const Exception *cause,...):Exception("",cause) {
	FOUNDATION_CORE_EXCEPTION_VARARGS(msg,cause);
}

void IllegalStateException::_onClone( Exception** exc ) const
{
	if ( !*exc )
	{
		*exc = new IllegalStateException( getMessage() );
	}
	Exception::_onClone( exc );
}

IllegalStateException::IllegalStateException( const IllegalStateException& e ) : Exception( e )
{

}