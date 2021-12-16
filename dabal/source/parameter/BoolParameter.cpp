#include <parameter/BoolParameter.h>
using parameter::BoolParameter;

FOUNDATION_CORE_OBJECT_TYPEINFO_IMPL(BoolParameter,BaseParameter);

BoolParameter::BoolParameter( const string& name, bool value ): BaseParameter( name ),
	mValue( value )
{
}