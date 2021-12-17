#pragma once
#include <parameter/BaseParameter.h>
using parameter::BaseParameter;
#include <parameter/UniqueParameterList.h>
using parameter::UniqueParameterList;

namespace parameter
{
	/**
	* Complex parameter based on uniqueParameterList
	*/
	class DABAL_API ComplexParameter : public BaseParameter, public UniqueParameterList
	{
		DABAL_CORE_OBJECT_TYPEINFO;
	public:
		ComplexParameter(){};
		ComplexParameter( const string& name );
	


	};

}

/*

	class DABAL_API ListParameter : public Parameter
	{
	public:
		ListParameter( );
		~ListParameter();
// 		/
// 		Sets the value of the parameter at the given index
// 		@param idx the index of the value to be set
// 		@param v the value to be set, whose ownership is taken by this parameter
// 		@return actual index at which the value was set (in case the given index is greater
// 		 than the current value list size)
// 		@throw IllegalArgumentException if the value being passed is not valid
// 		/
		size_t setValue(const unsigned int idx,Value* v);
		
// 		 Adds a new value to the parameter
// 		 @param v the value to be added, whose ownership is taken by this parameter
// 		 @throw IllegalArgumentException if the value being passed is not valid
		
		void addValue(Value* v);
		
		inline const ValueList& getValues() const;
		inline ValueList& getValues();
		inline Value* getValue( size_t idx );
		inline const Value* getValue( size_t idx ) const;

	private:
		ValueList mValues;
	protected:

		void _onClone( Parameter** self ) const;

	};
	const ValueList& ListParameter::getValues() const
	{
		return mValues;
	}
	ValueList& ListParameter::getValues()
	{
		return mValues;
	}
	Value* ListParameter::getValue( size_t idx )
	{
		return mValues[idx];
	}
	const Value* ListParameter::getValue( size_t idx ) const
	{
		return mValues[idx];
	}
	*/