#pragma once
#include <parameter/BaseParameter.h>
using parameter::BaseParameter;

namespace parameter
{
	class DABAL_API IntParameter : public BaseParameter
	{
		DABAL_CORE_OBJECT_TYPEINFO;
	public:
		IntParameter( const string& name, int value );
		inline void setValue( int value);
		inline int getValue() const;
	private:
		int mValue;
	};
	int IntParameter::getValue() const
	{
		return mValue;
	}
	void IntParameter::setValue( int value )
	{
		mValue = value;
	}
}