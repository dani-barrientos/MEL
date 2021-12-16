#include <parameter/IntParameter.h>
using parameter::IntParameter;

FOUNDATION_CORE_OBJECT_TYPEINFO_IMPL(IntParameter,BaseParameter);

IntParameter::IntParameter( const string& name, int value ): BaseParameter( name ),
	mValue( value )
{
}