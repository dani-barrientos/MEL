#include <parameter/SimpleParameterList.h>
using parameter::SimpleParameterList;
#include <core/Exception.h>
using core::Exception;

FOUNDATION_CORE_OBJECT_TYPEINFO_IMPL(SimpleParameterList,ParameterList);

bool SimpleParameterList::onAddParameter( BaseParameter* p )
{
	mParameters.push_back( p );
	return true;
}
void SimpleParameterList::onRemoveParameter( const BaseParameter* p )
{
	ListType::iterator i = mParameters.begin();
	while( i != mParameters.end() )
	{
		if ( i->getPtr() == p )
		{
			mParameters.erase( i );
			return;
		}
		++i;
	}

}
const BaseParameter* SimpleParameterList::onFindParameter( const char* name,bool throwException ) const
{
	string auxName = name;
	return findParameter( auxName,throwException );
}
const BaseParameter* SimpleParameterList::onFindParameter( const string& name,bool throwException ) const
{
	ListType::const_iterator pos = mParameters.begin();
	while ( pos != mParameters.end() )
	{
		if ( (*pos)->getName() == name )
			return pos->getPtr();
		++pos;
	}
	if ( throwException )
		throw Exception( "SimpleParameterList hasn't parameter %s",name.c_str());
	return NULL;
}

void SimpleParameterList::onMergeWith( ParameterList *other)
{
	if ( !other->getMyType().instanceOf( SimpleParameterList::type() ) )
		throw Exception( "SimpleParameterList::mergeWith. \'other\' list not a SimpleParameterList" );
	SimpleParameterList* otherAux = (SimpleParameterList*)other;
	//TODO hacer bien para que admita que sea virtual??
	for (ListType::iterator it=otherAux->getParameters().begin();it!=otherAux->getParameters().end();++it)
	{
		BaseParameter* parameterInOther=it->getPtr();
		const BaseParameter* parameterInOriginal = findParameter(parameterInOther->getName());
		if (parameterInOriginal!=NULL)
		{
/*			if (parameterInOriginal->getParamType()==Parameter::LIST && parameterInOther->getParamType()==Parameter::LIST)
				((ParamList*)parameterInOriginal)->mergeWith((ParamList*)parameterInOther);
			else
				addParameter(parameterInOther->clone());
				*/
			*it = parameterInOther;
		}
		else
			addParameter(parameterInOther);
	}
}