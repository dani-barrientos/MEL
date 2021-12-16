#pragma once
#include <parameter/ParameterList.h>
using parameter::ParameterList;
#include <list>
using std::list;
#include <core/SmartPtr.h>
using core::SmartPtr;
#include <map>
using std::map;

namespace parameter
{
	/**
	* ParameterList specialization for possibly repeated parameters. Internally use a list
	*/
	class FOUNDATION_API SimpleParameterList : public ParameterList
	{
		FOUNDATION_CORE_OBJECT_TYPEINFO;
	public:
		typedef list< SmartPtr<BaseParameter> >	ListType;
		inline const ListType& getParameters() const;
		inline ListType& getParameters();

	private:
		ListType	mParameters;
	protected:
		//! this version doen't matter about repeated params
		virtual bool onAddParameter( BaseParameter* p ) override;
		virtual void onRemoveParameter( const BaseParameter* p ) override;
		virtual const BaseParameter* onFindParameter( const char* name,bool throwException = false ) const override;
		virtual const BaseParameter* onFindParameter( const string& name,bool throwException = false  ) const override;
		void onMergeWith(ParameterList *other) override;

	};
	
	const SimpleParameterList::ListType& SimpleParameterList::getParameters() const
	{
		return mParameters;
	}
	SimpleParameterList::ListType& SimpleParameterList::getParameters()
	{
		return mParameters;
	}

}