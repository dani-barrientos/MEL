#include <parameter/FloatParameter.h>
using parameter::FloatParameter;

DABAL_CORE_OBJECT_TYPEINFO_IMPL(FloatParameter,BaseParameter);

FloatParameter::FloatParameter( const string& name, float value ): BaseParameter( name ),
	mValue( value )
{
}