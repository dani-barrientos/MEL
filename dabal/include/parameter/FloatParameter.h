#pragma once
#include <parameter/BaseParameter.h>
using parameter::BaseParameter;

namespace parameter
{
	class FOUNDATION_API FloatParameter : public BaseParameter
	{
		FOUNDATION_CORE_OBJECT_TYPEINFO;
	public:
		FloatParameter( const string& name, float value );
		inline void setValue( float value);
		inline float getValue() const;
	private:
		float mValue;
	};
	float FloatParameter::getValue() const
	{
		return mValue;
	}
	void FloatParameter::setValue( float value )
	{
		mValue = value;
	}

}