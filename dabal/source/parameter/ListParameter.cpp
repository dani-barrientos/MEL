#include <parameter/ListParameter.h>
using parameter::ListParameter;
#include <core/Exception.h>
using core::Exception;

DABAL_CORE_OBJECT_TYPEINFO_IMPL2(ListParameter,BaseParameter,SimpleParameterList);

ListParameter::ListParameter( const string& name ): BaseParameter( name )
{
}
