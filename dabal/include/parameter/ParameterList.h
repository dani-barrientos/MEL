#pragma once
#include <parameter/BaseParameter.h>
using parameter::BaseParameter;
#include <list>
using std::list;
#include <core/SmartPtr.h>
using core::SmartPtr;
#include <map>
using std::map;

namespace parameter
{
	/*
	* base class for parameter list
	* @todo poder iterar sobre la colección subyacente desde aquí
	*/
	class FOUNDATION_API ParameterList : virtual public ::core::IRefCount 
	{
		FOUNDATION_CORE_OBJECT_TYPEINFO;
	public:
		/**
		* 	add parameter. Takes ownership
		*	@return false if element already exists
		*/
		inline bool addParameter( BaseParameter* p );
		inline void removeParameter( const BaseParameter* p );
		/**
		* call onFindParameter.
		* @return the first Parameter found with given name
		*/
		inline const BaseParameter* findParameter( const char* name,bool throwException = false ) const;
		inline const BaseParameter* findParameter( const string& name,bool throwException = false  ) const;
		//! no-const version
		inline BaseParameter* findParameter( const char* name,bool throwException = false ) ;
		inline BaseParameter* findParameter( const string& name,bool throwException = false  );


		/**
		* Merges this ParamList with another ParamList
		* @param other the ParamList to merge with this
		* @note parameter are merge using smartptr, so no cloning is done
		*/
		//TODO hacer bien para que funcione con herencia
		inline void mergeWith(ParameterList *other);

	
	protected:
		//! this version doen't matter about repeated params
		virtual bool onAddParameter( BaseParameter* p ) = 0;
		virtual void onRemoveParameter( const BaseParameter* p ) = 0;
		virtual const BaseParameter* onFindParameter( const char* name,bool throwException = false ) const = 0;
		virtual const BaseParameter* onFindParameter( const string& name,bool throwException = false  ) const = 0;
		virtual void onMergeWith(ParameterList *other) = 0;

	};
	bool ParameterList::addParameter( BaseParameter* p )
	{
		return onAddParameter( p );
	}
	void ParameterList::removeParameter( const BaseParameter* p )
	{
		onRemoveParameter( p );
	}
	const BaseParameter* ParameterList::findParameter( const char* name,bool throwException ) const
	{
		return onFindParameter( name ,throwException );
	}
	const BaseParameter* ParameterList::findParameter( const string& name,bool throwException ) const
	{
		return onFindParameter( name ,throwException );
	}
	BaseParameter* ParameterList::findParameter( const char* name,bool throwException ) 
	{
		//este const_cast es chapucil para no tener que rehacer todos los hijos
		return const_cast<BaseParameter*>(const_cast<const ParameterList*>(this)->onFindParameter(name,throwException));
	}
	BaseParameter* ParameterList::findParameter( const string& name,bool throwException)
	{
		return const_cast<BaseParameter*>(const_cast<const ParameterList*>(this)->onFindParameter(name,throwException));
	}

	void ParameterList::mergeWith(ParameterList *other)
	{
		return onMergeWith( other );
	}


}