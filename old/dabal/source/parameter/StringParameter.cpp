#include <parameter/StringParameter.h>
using parameter::StringParameter;

DABAL_CORE_OBJECT_TYPEINFO_IMPL(StringParameter,BaseParameter);

StringParameter::StringParameter( const string& name, const string& value ): BaseParameter( name ),
		mValue( value )
{
}