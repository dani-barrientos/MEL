#include <core/Type.h>
#include <assert.h>
using std::string;

namespace core {

//Type::ReflectionMap Type::Reflection;
const Type Type::NOTYPE(NULL,NULL);

Type::ReflectionMap::ReflectionMap()
{
}

Type::ReflectionMap::~ReflectionMap()
{
}

void Type::ReflectionMap::registerType(const char * name, const Type * type)
{
	if (name && type)
	{
		mCS.enter();
		if(mReflectionMap.find(name) != mReflectionMap.end())
			;//Logger::getLogger()->warn("Type::ReflectionMap: Attempt to registerType '%s' already registered", name);	
		else
			mReflectionMap.insert(std::make_pair(name, type));
		mCS.leave();
	}
}

void Type::ReflectionMap::unregisterType(const char * name)
{
	if (name)
	{
		mCS.enter();
		std::map<string, const Type *>::iterator it = mReflectionMap.find(name);
		if (it!=mReflectionMap.end())
			mReflectionMap.erase(it);
		mCS.leave();
	}
}

const Type * Type::ReflectionMap::operator [](const char * name)
{
	const Type * value = NULL;

	mCS.enter();
	std::map<string, const Type*>::const_iterator it (mReflectionMap.find(name));
	if (it != mReflectionMap.end())
		value = it->second;
	mCS.leave();

	return value;
}


	//@todo el strdup seria interesante meterlo en StringUtils (pasarlas a foundation) para usar _strdup en Visual
Type::Type(const char* name,const Type *super):
	//mName(name?strdup( name ):NULL ), ¿strdup?
	mName( name ),
	mNumParents(1)
	
{
	mParent[0] = super;

	Type::Reflection().registerType(name, this);
}

Type::Type(const char* name,const Type* parent1,const Type* parent2) :
	mName( name ),	
	mNumParents(2)
{
	mParent[0] =  parent1;
	mParent[1] = parent2;
	
	Type::Reflection().registerType(name, this);
}
Type::Type(const char* name,const Type* parent1,const Type* parent2, const Type* parent3):
	mName( name ),
	mNumParents(3)
{
	mParent[0] = parent1;
	mParent[1] = parent2;
	mParent[2] = parent3;
	
	Type::Reflection().registerType(name, this);
}
Type::Type(const char* name,const Type* parent1,const Type* parent2, const Type* parent3,const Type* parent4) :
	mName( name ),
	mNumParents(4)
{
	mParent[0] = parent1;
	mParent[1] = parent2;
	mParent[2] = parent3;
	mParent[3] = parent4;
	
	Type::Reflection().registerType(name, this);
}

Type::Type(const char* name):
	//mName( strdup( name ) ),
	mName( name ),
	mNumParents(0)
{
	mParent[0] = 0;
	
	Type::Reflection().registerType(name, this);
}

Type::~Type() {

	Type::Reflection().unregisterType(mName);
	

	//free( (void*)mName );
	//OK. No hay nada más que hacer
}

/*
bool Type::instanceOf(const Type &t) const {
	const Type *h=this;
	bool is=false;
	while ((h!=NULL) && (!is)) {
		is=h==&t;
		h=h->mSuper;
	}
	return is;
}
*/
bool Type::instanceOf(const Type &t) const {

	//recorro los tipos de los padres del type comprobando si coincide
	if ( this == &t )
		return true;
	const Type* currentType;
	for(unsigned char i = 0; i < mNumParents; i++)
	{
		currentType = mParent[i];
		if ( !currentType )
			return false;
		if ( currentType->instanceOf( t ) )
			return true;
	}
	return false;

	
}
//end namespace
}