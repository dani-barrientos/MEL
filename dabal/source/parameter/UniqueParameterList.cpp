#include <parameter/UniqueParameterList.h>
using parameter::UniqueParameterList;
#include <core/Exception.h>
using core::Exception;

FOUNDATION_CORE_OBJECT_TYPEINFO_IMPL(UniqueParameterList,ParameterList);

UniqueParameterList::UniqueParameterList( const string& name )
{
}
bool UniqueParameterList::onAddParameter( BaseParameter* p )
{
	if ( mParameterMap.find( p->getName() ) != mParameterMap.end() )
		return false;
	else
	{
		mParameterMap.insert( ParameterCollection::value_type(p->getName(),p) );
		return true;
	}
}
void UniqueParameterList::onRemoveParameter( const BaseParameter* p )
{
	mParameterMap.erase( p->getName() );
}
const BaseParameter* UniqueParameterList::onFindParameter( const char* name,bool throwException ) const
{
	string auxName = name;
	return findParameter( auxName,throwException );
}
const BaseParameter* UniqueParameterList::onFindParameter( const string& name,bool throwException ) const
{
	ParameterCollection::const_iterator pos;
	pos = mParameterMap.find( name );
	if ( pos != mParameterMap.end() )
		return pos->second.getPtr();
	else
	{
		if (throwException)
			throw Exception( "UniqueParameterList hasn't parameter %s",name.c_str());
		else
			return NULL;
	}
	//it's not neccesary to call parent onFindParameter
}
void UniqueParameterList::onMergeWith( ParameterList *other)
{
	if ( !other->getMyType().instanceOf( UniqueParameterList::type() ) )
		throw Exception( "SimpleParameterList::mergeWith. \'other\' list not a SimpleParameterList" );
	UniqueParameterList* otherAux = (UniqueParameterList*)other;
	//TODO hacer bien para que admita que sea virtual??
	for (ParameterCollection::iterator it=otherAux->getParameters().begin();it!=otherAux->getParameters().end();++it)
	{
		BaseParameter* parameterInOther = it->second.getPtr();
		const BaseParameter* parameterInOriginal = findParameter(parameterInOther->getName());
		if (parameterInOriginal!=NULL)
		{
/*			if (parameterInOriginal->getParamType()==Parameter::LIST && parameterInOther->getParamType()==Parameter::LIST)
				((ParamList*)parameterInOriginal)->mergeWith((ParamList*)parameterInOther);
			else
				addParameter(parameterInOther->clone());
				*/
			it->second = parameterInOther;
		}
		else
			addParameter(parameterInOther);
	}
}

