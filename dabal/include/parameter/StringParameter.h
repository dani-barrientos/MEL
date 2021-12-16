#pragma once
#include <parameter/BaseParameter.h>
using parameter::BaseParameter;

namespace parameter
{
	/**
	* TODO
	*/
	class FOUNDATION_API StringParameter : public BaseParameter
	{
		FOUNDATION_CORE_OBJECT_TYPEINFO;
	public:
		StringParameter( const string& name, const string& value );
		inline void setValue( const char* value );
		inline void setValue( const string& value );
		inline const string& getValue() const;
	private:
		string mValue;
	};
	const string& StringParameter::getValue() const
	{
		return mValue;
	}
	void StringParameter::setValue( const char* value )
	{
		mValue = value;
	}
	void StringParameter::setValue( const string& value )
	{
		mValue = value;
	}
}