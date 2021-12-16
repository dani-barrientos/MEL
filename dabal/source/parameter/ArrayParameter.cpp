#include <parameter/ArrayParameter.h>
using parameter::ArrayParameter;

FOUNDATION_CORE_OBJECT_TYPEINFO_IMPL(ArrayParameter,BaseParameter);

ArrayParameter::ArrayParameter( const string& name, size_t tam ): BaseParameter( name ),
	mValues(tam)
{
}