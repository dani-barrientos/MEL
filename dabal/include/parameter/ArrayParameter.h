#pragma once
#include <parameter/BaseParameter.h>
using parameter::BaseParameter;
#include <vector>
#include <core/SmartPtr.h>
namespace parameter
{
	using std::vector;
	using ::core::SmartPtr;
	class DABAL_API ArrayParameter : public BaseParameter
	{
		DABAL_CORE_OBJECT_TYPEINFO;
	public:
		typedef vector<SmartPtr<BaseParameter> > ArrayType;

		ArrayParameter( const string& name, size_t sizes);

		inline size_t getCount() const;
		//! set parameter for given index. Size is not checked!
		inline void setValue( size_t idx, BaseParameter* value );
		//! get parameter for given index. Size is not checked!
		inline const BaseParameter* getValue( size_t idx ) const;
		inline BaseParameter* getValue( size_t idx );
		inline void addValue( BaseParameter* value );
		inline const ArrayType& getValues() const;
		inline ArrayType& getValues();
	private:
		ArrayType	mValues;		
	};
	void ArrayParameter::setValue( size_t idx, BaseParameter* value )
	{
		mValues[idx] = value;
	}
	const BaseParameter* ArrayParameter::getValue( size_t idx ) const
	{
		return mValues[idx];
	}
	BaseParameter* ArrayParameter::getValue( size_t idx )
	{
		return mValues[idx];
	}
	const ArrayParameter::ArrayType& ArrayParameter::getValues() const
	{
		return mValues;
	}
	ArrayParameter::ArrayType& ArrayParameter::getValues()
	{
		return mValues;
	}
	void ArrayParameter::addValue( BaseParameter* value )
	{
		mValues.push_back( value );
	}
	size_t ArrayParameter::getCount() const
	{
		return mValues.size();
	}
}