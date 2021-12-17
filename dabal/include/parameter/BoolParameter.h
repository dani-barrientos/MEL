#pragma once
#include <parameter/BaseParameter.h>
using parameter::BaseParameter;

namespace parameter
{
	class DABAL_API BoolParameter : public BaseParameter
	{
		DABAL_CORE_OBJECT_TYPEINFO;
	public:
		BoolParameter( const string& name, bool value );
		inline void setValue( bool value);
		inline bool getValue() const;
	private:
		bool mValue;
	};
	bool BoolParameter::getValue() const
	{
		return mValue;
	}
	void BoolParameter::setValue( bool value )
	{
		mValue = value;
	}
}