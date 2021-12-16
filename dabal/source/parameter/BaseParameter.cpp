#include <parameter/BaseParameter.h>
using parameter::BaseParameter;

FOUNDATION_CORE_OBJECT_TYPEINFO_IMPL_ROOT(BaseParameter);

BaseParameter::BaseParameter( const string& name ):
	mName( name )
{
}