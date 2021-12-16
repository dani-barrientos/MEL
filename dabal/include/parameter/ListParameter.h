#pragma once
#include <parameter/BaseParameter.h>
using parameter::BaseParameter;
#include <parameter/SimpleParameterList.h>
using parameter::SimpleParameterList;

namespace parameter
{
	/**
	* Complex parameter based on SimpleParameterList (so with possibly repeated params)
	*/
	class FOUNDATION_API ListParameter : public BaseParameter, public SimpleParameterList
	{
		FOUNDATION_CORE_OBJECT_TYPEINFO;
	public:
		ListParameter(){};
		ListParameter( const string& name );
	


	};

}
