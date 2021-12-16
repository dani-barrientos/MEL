#include <core/UnknownException.h>
using core::UnknownException;

FOUNDATION_CORE_OBJECT_TYPEINFO_IMPL(UnknownException,Exception)

UnknownException::UnknownException():Exception("")
{}

UnknownException::UnknownException( const UnknownException& e ):Exception(e) {
}

void UnknownException::throwSame() const {
	throw UnknownException(*this);
}

void UnknownException::_onClone(Exception** exc ) const {
	if ( !*exc ) {
		*exc = new UnknownException();
	}
	Exception::_onClone( exc );
}