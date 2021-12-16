#include <parameter/StringParameter.h>
using parameter::StringParameter;

FOUNDATION_CORE_OBJECT_TYPEINFO_IMPL(StringParameter,BaseParameter);

StringParameter::StringParameter( const string& name, const string& value ): BaseParameter( name ),
		mValue( value )
{
}