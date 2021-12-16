#include <parameter/IRefCountParameter.h>
using parameter::IRefCountParameter;

FOUNDATION_CORE_OBJECT_TYPEINFO_IMPL(IRefCountParameter, BaseParameter);

IRefCountParameter::IRefCountParameter(const string& name, IRefCount* value) : BaseParameter(name),
mObject(value)
{
}