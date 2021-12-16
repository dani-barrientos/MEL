#include <parameter/ComplexParameter.h>
using parameter::ComplexParameter;
#include <core/Exception.h>
using core::Exception;

FOUNDATION_CORE_OBJECT_TYPEINFO_IMPL(ComplexParameter,BaseParameter);

ComplexParameter::ComplexParameter( const string& name ): BaseParameter( name )
{
}




/*
SimpleParameter::SimpleParameter( ):Parameter( Parameter::SIMPLE),mValue( 0 )
{
};

SimpleParameter::~SimpleParameter()
{
	delete mValue;
}
void SimpleParameter::_onClone( Parameter** self ) const
{
	if ( *self == NULL )
		*self = new SimpleParameter;
	static_cast<SimpleParameter*>( *self )->setValue( mValue->clone() );
	Parameter::_onClone( self );
}

//class ListParameter
ListParameter::ListParameter(  ):Parameter( Parameter::LIST )
{
	mValues.setOwner( this );
}
ListParameter::~ListParameter()
{
}

void ListParameter::addValue( Value* v ) 
{
	mValues.getValues().push_back(v); 
}

size_t ListParameter::setValue(const unsigned int idx,Value* v) 
{
	if (mValues.size()>idx) {
		//Parameter taks ownership of the values whithin it, so
		//we must delete de previous value
		delete mValues[idx];
		mValues[idx]=v;
		return idx;
	}
	else {
		mValues.getValues().push_back(v);
		return mValues.size()-1;
	}
}
void ListParameter::_onClone( Parameter** self ) const
{
	if ( *self == NULL )
		*self = new ListParameter;
	ListParameter* tmp = static_cast<ListParameter*>(*self);
	tmp->mValues.mContent.resize( mValues.mContent.size() );
	for( size_t i = 0 ; i < mValues.mContent.size(); i++ )
	{
		tmp->mValues.mContent[i] = mValues.mContent[i]->clone();
	}
	tmp->mValues.setOwner( tmp );
	Parameter::_onClone( self );
}
*/