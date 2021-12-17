#pragma once
#include <DabalLibType.h>
#include <core/IRefCount.h>
#include <string>
using std::string;
namespace parameter
{
	/**
	* TODO
	*/
	class DABAL_API BaseParameter : virtual public ::core::IRefCount
	{
		DABAL_CORE_OBJECT_TYPEINFO;
	public:
		BaseParameter(){};
		BaseParameter( const string& name );
		inline void setName( const char* name );
		inline void setName( const string& name );

		inline const string& getName() const;
	private:
		string mName;
	};
	void BaseParameter::setName( const char* name )
	{
		mName = name;
	}
	void BaseParameter::setName( const string& name )
	{
		mName = name;
	}
	const string& BaseParameter::getName() const
	{
		return mName;
	}
}