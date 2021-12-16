#ifdef _ANDROID
#include <java/jnibindings.h>
#include <core/Exception.h>
#include <core/CriticalSection.h>
#include <core/Thread.h>
#include <logging/Logger.h>
using logging::Logger;

#include <cstdio>
#include <map>

using java::Class;
using namespace java;

typedef core::CKHashMap<Class*> ClassMap;

JavaVM* java::VM(nullptr);
static ClassMap CLASSES(true,nullptr);
static core::CriticalSection* mCS(nullptr);
static jobject mClassLoader(nullptr);
static Method* mFindClass(nullptr);

static jclass _forName(const char* name,JNIEnv* jni) {
	jclass clazz(nullptr);
	if (mClassLoader && mFindClass) {
		//Try with the custom classloader
		clazz = mFindClass->call<jclass>(mClassLoader, "@", name);
	}
	if (!clazz) {
		//Try with the default class finding method
		java::exceptionClear(false, jni);
		std::string cName(name);
		std::replace(cName.begin(), cName.end(), '.', '/');
		clazz=jni->FindClass(cName.c_str());
	}

	if (!clazz) {
		java::exceptionClear(true, jni);
	}
	return clazz;
}

void java::init(JavaVM* avm,jobject instance) {
	if (!avm)
		throw core::Exception("Unable to initialize Java bindings with NULL JavaVM");

	if (VM) {
		if (VM!=avm)
			throw core::Exception("Trying to initialize Java bindings with more than one JavaVM instance");
		else
			return;
	}

	CLASSES.clear();
	VM=avm;	
	mCS=new core::CriticalSection();

	try {
		JNIEnv* jni(getJNI());
		Class* cClass(Class::forName("java.lang.Class"));
		Method* getClassLoader(cClass->getMethod("getClassLoader", "()Ljava/lang/ClassLoader;"));

		jclass instanceClass(instance ? jni->GetObjectClass(instance) : cClass->getClass());

		Class* cClassLoader(Class::forName("java.lang.ClassLoader"));
		mFindClass = cClassLoader->getMethod("findClass", "(Ljava/lang/String;)Ljava/lang/Class;");

		jobject classLoader = getClassLoader->call<jobject>(instanceClass, NULL);
		if (classLoader) {
			mClassLoader=jni->NewGlobalRef(classLoader);
			jni->DeleteLocalRef(classLoader);
			Class::forName("java.lang.Object");
			Class::forName("java.lang.String");
		}

		//Clear any pending exception (and throw native if any)
		exceptionClear(true, jni);

		//Delete local refs
		if (instance) {
			jni->DeleteLocalRef(instanceClass);
		}
	}
	catch (Exception&e) {
		Logger::getLogger()->error("java::init:failed", &e);
	}
}

static void _detachJVM(core::Thread* thread);
static java::DetachResult detachCurrentThreadInternal(bool unsubscribe) {
	if (java::VM) {
		JNIEnv* jni(nullptr);
		jint r = VM->GetEnv((void **)&jni, JNI_VERSION_1_2);
		if (r == JNI_OK) {
			r = java::VM->DetachCurrentThread();
			if (r == JNI_OK) {
				if (unsubscribe) {
					core::Thread* t(core::Thread::getCurrentThread());
					if (t) {
						t->unsubscribeCallback(_detachJVM);
					}
				}
				return DR_DETACHED;

			}
			else {
				return DR_ERROR;
			}
		}
		else {
			return DR_NOT_ATTACHED;
		}
	}
	else {
		return DR_NOT_ATTACHED;
	}
}

static void _detachJVM(core::Thread* thread) {
	detachCurrentThreadInternal(false);
}

java::DetachResult java::detachCurrentThread() {
	return detachCurrentThreadInternal(true);
}

JNIEnv* java::attachCurrentThread() {
	JNIEnv* jni(nullptr);
	jint r(java::VM->AttachCurrentThread(&jni, nullptr));
	return (r == JNI_OK) ? jni : nullptr;
}

JNIEnv* java::getJNI() {
	JNIEnv* jni=NULL;
	if (VM) {
		jint r=VM->GetEnv((void **)&jni, JNI_VERSION_1_2);
		if (r!=JNI_OK) {
			r=java::VM->AttachCurrentThread(&jni,NULL);
			if (r!=JNI_OK) {
				jni=NULL;
			}
			else {
				core::Thread* thread=core::Thread::getCurrentThread();
				if (thread)
					thread->subscribeCallback(_detachJVM);
			}
		}
	}

	if (!jni)
		throw core::Exception("Unable to get JNI environment!");

	return jni;
}

void java::exceptionClear(bool throwNative,JNIEnv* jni) {
	if (!jni) jni = getJNI();
	if (throwNative) {
		jthrowable t(jni->ExceptionOccurred());
		jni->ExceptionClear();
		if (t) {
			string msg;
			try {
				Class* cThrowable = Class::registerClass(jni->GetObjectClass(t));
				{
					Method* getClass(Class::forName("java.lang.Object")->getMethod("getClass", "()Ljava/lang/Class;"));
					Method* getName(Class::forName("java.lang.Class")->getMethod("getName", "()Ljava/lang/String;"));
					jstring className(getName->call<jstring>(getClass->call<jobject>(t)));
					const char* tmp(jni->GetStringUTFChars(className, nullptr));
					msg.append(tmp?tmp:"").append(":");
					jni->ReleaseStringUTFChars(className, tmp);
					jni->DeleteLocalRef(className);
				}
				{
					jstring message = cThrowable->getMethod("getMessage", "()Ljava/lang/String;")->call<jstring>(t);
					const char* tmp(jni->GetStringUTFChars(message, nullptr));
					msg.append(tmp ? tmp : "");
					jni->ReleaseStringUTFChars(message, tmp);
					jni->DeleteLocalRef(message);
				}
			}
			catch (core::Exception &e) {
				throw Exception("java::exceptionClear:failed to get exception information",&e);
			}

			throw Exception(msg.c_str());
		}
	}
	else {
		//No need to throw native, just clear the exception
		if (jni->ExceptionCheck()) {
			jni->ExceptionClear();
		}
	}	
}

void java::shutdown() {
	if (mCS)
		mCS->enter();
	CLASSES.clear();
	if (mClassLoader) {
		JNIEnv* jni(getJNI());
		jni->DeleteGlobalRef(mClassLoader);
		mClassLoader = nullptr;
	}
	if (mCS)
		mCS->leave();

	VM=nullptr;
	delete mCS;
	mCS=nullptr;
}

Class::Class(const char* name,jclass clazz):	
	mMethods(true,NULL) {
	if (!VM)
		throw core::Exception("No JavaVM set yet!");
	
	JNIEnv* jni(getJNI());
	bool ownLocalRef=!clazz;
	if (!clazz) {
		clazz = _forName(name,jni);
	}

	if (!clazz)
		throw core::Exception("Unable to find Java class %s",name);
	
	//Nos quedamos con una referecia global
	mName=name;
	mClass=(jclass)jni->NewGlobalRef(clazz);
	//Ya no necesitamos la referenciar local (pero solo
	//la destruimos si la tuvimos que crear nosotros)
	if (ownLocalRef)
		jni->DeleteLocalRef(clazz);
}

Class::~Class() {
	JNIEnv* jni=getJNI();
	mMethods.clear();
	if (jni && mClass) {			
		jni->DeleteGlobalRef(mClass);
		mClass=NULL;
	}
}

Class* Class::forName(const char* name) {
	mCS->enter();
	Class* clazz=CLASSES.get(name);
	if (!clazz) {		
		CLASSES.put(name,clazz=new Class(name));
	}
	mCS->leave();
	return clazz;
}

Class* Class::registerClass(jclass clazz) {
	JNIEnv* jni=getJNI();

	Method* getName=forName("java.lang.Class")->getMethod("getName","()Ljava/lang/String;");
	jstring name=(jstring)jni->CallObjectMethod(clazz,getName->getID());
	if (!name)
		throw core::Exception("Unable to get class name");

	const char* nameChars=jni->GetStringUTFChars(name,NULL);
	Class* cls=NULL;
	if (nameChars) {
		mCS->enter();
		cls=CLASSES.get(nameChars);
		if (!cls) {
			CLASSES.put(nameChars,cls=new Class(nameChars,clazz));
		}
		mCS->leave();
	}
	jni->ReleaseStringUTFChars(name,nameChars);	
	//Liberar referencia local!
	if (name)
		jni->DeleteLocalRef(name);

	if (!cls)
		throw core::Exception("Unable to register class");
	return cls;
}

Method* Class::getMethod(const char* name,const char* signature) {
	if (!signature) signature="()V";
	
	char tmp[256];
	tmp[0]=0;
	strncat(tmp,name,256);
	strncat(tmp,signature,256);

	mCS->enter();
	Method* method=mMethods.get(tmp);
	
	if (!method) {		
		JNIEnv* jni=getJNI();
		jmethodID mID=jni?jni->GetMethodID(mClass,name,signature):NULL;
		if (!mID) {
			mCS->leave();
			exceptionClear(true, jni);
			throw core::Exception("Method not found: %s.%s%s",mName.c_str(),name,signature);
		}
		mMethods.put(tmp,method=new Method(mID,this,false));
	}
	mCS->leave();

	return method;
}

Method* Class::getStaticMethod(const char* name,const char* signature) {
	if (!signature) signature="()V";
	
	char tmp[256];
	tmp[0]=0;
	strncat(tmp,name,256);
	strncat(tmp,signature,256);

	mCS->enter();
	Method* method=mMethods.get(tmp);
	
	if (!method) {		
		JNIEnv* jni=getJNI();
		jmethodID mID=jni?jni->GetStaticMethodID(mClass,name,signature):NULL;
		if (!mID) {
			mCS->leave();
			exceptionClear(true, jni);
			throw core::Exception("Static method not found: %s.%s%s",mName.c_str(),name,signature);
		}
		mMethods.put(tmp,method=new Method(mID,this,false));
	}
	mCS->leave();

	return method;
}

Method* Class::getConstructor(const char* signature) {
	Method* m=getMethod("<init>",signature);
	if (m)
		m->mConstructor=true;
	return m;
}

Method::ParameterList::ParameterList(JNIEnv* _jni,const char* types,va_list values):mJNI(_jni),mValues(NULL) {

	int lng=types?strlen(types):0;
	if (!lng)
		return;

	void* ptr;

	mValues=new jvalue[lng];
	for (int i=0;i<lng;++i,++types) {
		char c=*types;
		switch (c) {
			//boolean
			case 'z':
				mValues[i].z=va_arg(values,int)?JNI_TRUE:JNI_FALSE;
				break;
			case 'Z':
				mValues[i].z=va_arg(values,int);
				break;

			//byte
			case 'b':				
			case 'B':
				mValues[i].b=va_arg(values,int);
				break;
			
			//char
			case 'c':
				mValues[i].c=va_arg(values,int);
				break;
			case 'C':
				mValues[i].c=va_arg(values,int);
				break;
			
			//int
			case 'i':
				mValues[i].i=va_arg(values,int);
				break;
			case 'I':
				mValues[i].i=va_arg(values,jint);
				break;
			
			//long
			case 'j':
				mValues[i].j=va_arg(values,long long);
				break;
			case 'J':
				mValues[i].j=va_arg(values,jlong);
				break;

			//float
			case 'f':
				mValues[i].f=va_arg(values,double);
				break;
			case 'F':
				mValues[i].f=va_arg(values,double);
				break;

			//double
			case 'd':
				mValues[i].d=va_arg(values,double);
				break;
			case 'D':
				mValues[i].d=va_arg(values,jdouble);
				break;

			//object instance
			case 'o':
				mValues[i].l=NULL;//!@todo: java::Object
				break;
			case 'O':
				mValues[i].l=va_arg(values,jobject);
				break;
			
			//C string -> java.lang.String
			case '@':
				ptr=va_arg(values,void*);
				mValues[i].l=ptr?mJNI->NewStringUTF((char*)ptr):NULL;
				mLocalRefs.push_back(mValues[i].l);
				break;

			default:
				throw core::Exception("Invalid parameter type %c",c);
		}
	}
}

Method::ParameterList::~ParameterList() {
	unsigned int lcount=mLocalRefs.size();
	if (lcount) {		
		for (int i=0;i<lcount;++i) {
			mJNI->DeleteLocalRef(mLocalRefs[i]);
		}
		mLocalRefs.clear();
	}

	if (mValues) {
		delete mValues;
		mValues=NULL;
	}
}

template<> void Method::call<void>(jobject instance,const char* paramTypes,...) {
	if (mConstructor)
		throw core::Exception("Invalid constructor call");

	JNIEnv* jni=getJNI();

	va_list args;
	va_start(args,paramTypes);
	ParameterList pl(jni,paramTypes,args);

	if (instance) {
		pl.getValues()?
			jni->CallVoidMethodA(instance,mID,pl.getValues()):
			jni->CallVoidMethodV(instance,mID,args);
	}
	else {
		pl.getValues()?
			jni->CallStaticVoidMethodA(mClass->getClass(),mID,pl.getValues()):
			jni->CallStaticVoidMethodV(mClass->getClass(),mID,args);
	}	
	va_end(args);
	java::exceptionClear(true, jni);
}

template<> jobject Method::call<jobject>(jobject instance,const char* paramTypes,...) {
	JNIEnv* jni=getJNI();

	va_list args;
	va_start(args,paramTypes);

	ParameterList pl(jni,paramTypes,args);
	
	jobject result;
	if (mConstructor)
		result=pl.getValues()?
				jni->NewObjectA(mClass->getClass(),mID,pl.getValues()):
				jni->NewObjectV(mClass->getClass(),mID,args);
	else {
		if (instance) {
			result=pl.getValues()?
				jni->CallObjectMethodA(instance,mID,pl.getValues()):
				jni->CallObjectMethodV(instance,mID,args);
		}
		else {
			result=pl.getValues()?
				jni->CallStaticObjectMethodA(mClass->getClass(),mID,pl.getValues()):
				jni->CallStaticObjectMethodV(mClass->getClass(),mID,args);
		}
	}

	va_end(args);
	java::exceptionClear(true, jni);

	return result;
}

template<> jlong Method::call<jlong>(jobject instance,const char* paramTypes,...) {
	if (mConstructor)
		throw ::core::Exception("Invalid constructor call");

	JNIEnv* jni=getJNI();

	va_list args;
	va_start(args,paramTypes);	
	ParameterList pl(jni,paramTypes,args);

	jlong result;	
	if (instance) {
		result=pl.getValues()?
			jni->CallLongMethodA(instance,mID,pl.getValues()):
			jni->CallLongMethodV(instance,mID,args);
	}
	else {
		result=pl.getValues()?
			jni->CallStaticLongMethodA(mClass->getClass(),mID,pl.getValues()):
			jni->CallStaticLongMethodV(mClass->getClass(),mID,args);
	}

	va_end(args);
	java::exceptionClear(true, jni);

	return result;
}

template<> jboolean Method::call<jboolean>(jobject instance,const char* paramTypes,...) {
	if (mConstructor)
		throw ::core::Exception("Invalid constructor call");
	
	JNIEnv* jni=getJNI();
	va_list args;
	va_start(args,paramTypes);
	ParameterList pl(jni,paramTypes,args);

	jboolean result;	
	if (instance) {
		result=pl.getValues()?
			jni->CallBooleanMethodA(instance,mID,pl.getValues()):
			jni->CallBooleanMethodV(instance,mID,args);
	}
	else {
		result=pl.getValues()?
			jni->CallStaticBooleanMethodA(mClass->getClass(),mID,pl.getValues()):
			jni->CallStaticBooleanMethodV(mClass->getClass(),mID,args);
	}

	va_end(args);
	java::exceptionClear(true, jni);

	return result;
}


template<> jint Method::call<jint>(jobject instance,const char* paramTypes,...) {
	if (mConstructor)
		throw ::core::Exception("Invalid constructor call");

	JNIEnv* jni=getJNI();
	va_list args;
	va_start(args,paramTypes);
	ParameterList pl(jni,paramTypes,args);

	jint result;	
	if (instance) {
		result=pl.getValues()?
			jni->CallIntMethodA(instance,mID,pl.getValues()):
			jni->CallIntMethodV(instance,mID,args);
	}
	else {
		result=pl.getValues()?
			jni->CallStaticIntMethodA(mClass->getClass(),mID,pl.getValues()):
			jni->CallStaticIntMethodV(mClass->getClass(),mID,args);
	}

	va_end(args);
	java::exceptionClear(true, jni);

	return result;
}

template<> jstring Method::call<jstring>(jobject instance, const char* paramTypes, ...) {
	JNIEnv* jni = getJNI();

	va_list args;
	va_start(args, paramTypes);

	ParameterList pl(jni, paramTypes, args);

	jobject result;
	if (mConstructor)
		result = pl.getValues() ?
		jni->NewObjectA(mClass->getClass(), mID, pl.getValues()) :
		jni->NewObjectV(mClass->getClass(), mID, args);
	else {
		if (instance) {
			result = pl.getValues() ?
				jni->CallObjectMethodA(instance, mID, pl.getValues()) :
				jni->CallObjectMethodV(instance, mID, args);
		}
		else {
			result = pl.getValues() ?
				jni->CallStaticObjectMethodA(mClass->getClass(), mID, pl.getValues()) :
				jni->CallStaticObjectMethodV(mClass->getClass(), mID, args);
		}
	}

	va_end(args);
	java::exceptionClear(true, jni);

	return (jstring)result;
}

template<> jclass Method::call<jclass>(jobject instance, const char* paramTypes, ...) {
	JNIEnv* jni = getJNI();

	va_list args;
	va_start(args, paramTypes);

	ParameterList pl(jni, paramTypes, args);

	jobject result;
	if (mConstructor)
		result = pl.getValues() ?
		jni->NewObjectA(mClass->getClass(), mID, pl.getValues()) :
		jni->NewObjectV(mClass->getClass(), mID, args);
	else {
		if (instance) {
			result = pl.getValues() ?
				jni->CallObjectMethodA(instance, mID, pl.getValues()) :
				jni->CallObjectMethodV(instance, mID, args);
		}
		else {
			result = pl.getValues() ?
				jni->CallStaticObjectMethodA(mClass->getClass(), mID, pl.getValues()) :
				jni->CallStaticObjectMethodV(mClass->getClass(), mID, args);
		}
	}

	va_end(args);
	//Only throw exception if we are not finding classes via _forName!
	java::exceptionClear((this != mFindClass) || (instance != mClassLoader), jni);

	return (jclass)result;
}

Object::Object(jobject instance,bool localRef):
	mInstance(instance) {	
	JNIEnv* jni=getJNI();
	
	try {
		Class* obj=Class::forName("java.lang.Object");
		assert(obj && "Unable to get class java.lang.Object");

		assert(Class::forName("java.lang.Class"));

		Method* getClass=obj->getMethod("getClass","()Ljava/lang/Class;");
		if (!getClass)
			throw core::Exception("Unable to get class java.lang.Object.getClass()");
	
		jclass clazz=(jclass)getClass->call<jobject>(instance);
		if (!clazz)
			throw core::Exception("Unable to invole java.lang.Object.getClass()");

		mClass=Class::registerClass(clazz);
		//Liberamos referencia local (se supone que al registar la clase se crea una
		//global que nos asegura que la clase como tal no se libera hasta que no
		//se invoke java::shutdown
		jni->DeleteLocalRef(clazz);
		if (!mClass) {
			throw core::Exception("Unable to register class");
		}

		//Todo fue OK, así que nos reservamos una referencia global para
		//que todo funcione bien mientras viva esta instancia
		mInstance=jni->NewGlobalRef(instance);
		if (!mInstance)
			throw core::Exception("Unable to create global reference to instance!");

		//Liberar la referencia local (si asi se indico en el constructor)
		if (localRef)
			jni->DeleteLocalRef(instance);
	}
	catch (...) {
		if (localRef)
			jni->DeleteLocalRef(instance);
		throw;
	}
}

Object::~Object() {
	JNIEnv* jni=getJNI();
	if (mClass && mInstance && jni) {
		jni->DeleteGlobalRef(mInstance);
	}
	mClass=NULL;
	mInstance=NULL;
}

Method* Object::getMethod(const char* name,const char* signature/* =NULL */) {
	return mClass->getMethod(name,signature);
}
#endif