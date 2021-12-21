#pragma once
#include <parameter/ParameterList.h>
using parameter::ParameterList;

namespace parameter
{
	/**
	* Parameter list spezilization for no repeated parameters, so
	* can eficiently be accessed by name (internally use a map)
	*/
	class DABAL_API UniqueParameterList :  public ParameterList
	{
		DABAL_CORE_OBJECT_TYPEINFO;
	public:
		UniqueParameterList(){};
		UniqueParameterList( const string& name );

		typedef map< string,SmartPtr<BaseParameter> > ParameterCollection;
		inline const ParameterCollection& getParameters() const;
		inline ParameterCollection& getParameters();

	private:
		ParameterCollection	mParameterMap;
	protected:
		//!overridden from ParameterList
		virtual bool onAddParameter( BaseParameter* p ) override;
		//!overridden from ParameterList
		virtual void onRemoveParameter( const BaseParameter* p ) override;
		//!overridden from ParameterList
		virtual const BaseParameter* onFindParameter( const char* name,bool throwException = false ) const override;
		//!overridden from ParameterList
		virtual const BaseParameter* onFindParameter( const string& name,bool throwException = false  ) const override;
		//!overridden from ParameterList
		void onMergeWith(ParameterList *other) override;
	};
	const UniqueParameterList::ParameterCollection& UniqueParameterList::getParameters() const
	{
		return mParameterMap;
	}
	UniqueParameterList::ParameterCollection& UniqueParameterList::getParameters()
	{
		return mParameterMap;
	}


}
