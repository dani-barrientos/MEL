#pragma once
#include <parameter/BaseParameter.h>
using parameter::BaseParameter;
#include <core/IRefCount.h>
using ::core::IRefCount;
#include <core/SmartPtr.h>
namespace parameter
{
	/**
	*hold IRefCount object as SmartPtr
	*/
	class FOUNDATION_API IRefCountParameter : public BaseParameter
	{
		FOUNDATION_CORE_OBJECT_TYPEINFO;
	public:
		IRefCountParameter(const string& name, IRefCount* value);
		inline void setValue(IRefCount* value);
		inline IRefCount* getValue() const;
	private:
		::core::SmartPtr<IRefCount> mObject;
	};
	IRefCount* IRefCountParameter::getValue() const
	{
		return mObject;
	}
	void IRefCountParameter::setValue(IRefCount* value)
	{
		mObject = value;
	}
}
