#pragma once
#include <jni.h>
#include <core/Exception.h>
#include <core/HashTypes.h>
#include <vector>

namespace java {
	
	extern JavaVM* VM;	

	enum DetachResult {
		DR_DETACHED=0,		//Thread detached successfuly
		DR_NOT_ATTACHED,	//Thread was not attached; no neeed to detach
		DR_ERROR			//Thread was attached, but couldn't be detached
	};
	/*
	* Intialize the jni framework
	* @param vm the JVM to init the framework with
	* @param classInstance an optional instance to grab the ClassLoader from, to be
	* used for subsequent class finding
	* @throw Exception if vm is NULL, or the framework has already been initialized with
	* a different JVM
	*/
	void init(JavaVM* vm,jobject instance=nullptr);

	/**
	* Get a new JNIEnv on the current thread.
	* @return a valid JNIEnv pointer ready to be used. If the calling
	* thread was not yet attached to the JVM, it's automatically attached
	* and scheduled to be detached as soon as the thread terminates. Clients
	* can manually dettach the VM if they wish at any time (thus, cancelling
	* the detaching upon thread termination)
	* @throw Exception if a valid JNIEnv cannot be retrieved
	*/
	JNIEnv* getJNI();

	/**
	* Clear pending exception (if any) in the JVM
	* @param throwNative `true` to force a native exception to be thrown if
	* the JVM had a pending exception. `false` so silently ignore that fact.
	* @param optional JNIEnv pointer (passing null will auto-generate a temporary one)
	*/
	void exceptionClear(bool throwNative,JNIEnv* jni=NULL);

	/**
	* Detaches the current thread from the VM.
	* If the thread has never been attached, or is already detahed, this
	* is a no-op.
	* @return the result of the detach operation
	*/
	DetachResult detachCurrentThread();

	/**
	* Attaches the current thread to the VM
	* @return a valid JNIEnv pointer, or nullptr if the attach fails
	*/
	JNIEnv* attachCurrentThread();

	/**
	* Frees up any extra resources and shuts down the jni framework
	*/
	void shutdown();
	
	class Class;
	class Object;
	class Method {
		friend class Class;
		public:
			inline jmethodID getID() const {return mID;}	
			/**
			* @param instancia instancia del objeto sobre la que se quiere invocar
			* el metodo (NULL para metodos estáticos)
			* @param paramType cadena que contiene el tipo de los datos nativos (C++)
			* de cada parametro, o NULL si los parametros enviados ya sin tipos JNI.
			* Se sigue la misma nomenclatura que en JNI, pero en minúsculas (z=boolean/Z=jboolean,etc),
			* con excepción de que los parametros de tipo "objeto" que se identicarán como "O" si son
			* jobject ú "o" si son de tipo java::Object*.
			* Permite pasar cadenas en C nativo con el identificador de tipo "@". Cuando se
			* encuentra este tipo, se construye internamente un objeto java.lang.String temporal,
			* cuya referencia se libera mientras dura la llamada
			*/			
			template<class T> T call(jobject instance,const char* paramType=NULL,...);

		private:
			class ParameterList {
				friend class Method;
				public:
					inline jvalue* getValues() {return mValues;}
				private:
					ParameterList(JNIEnv* jni,const char* types,va_list values);
					~ParameterList();
					
					JNIEnv* mJNI;					
					jvalue* mValues;
					std::vector<jobject> mLocalRefs;
			};

			Method(jmethodID id,Class* clazz,bool constructor=false):mClass(clazz),mID(id),mConstructor(constructor) {}

			Class* mClass;
			jmethodID mID;
			bool mConstructor;
	};
	template<> void Method::call<void>(jobject instance,const char* paramTypes,...);
	template<> jobject Method::call<jobject>(jobject instance,const char* paramTypes,...);
	template<> jlong Method::call<jlong>(jobject instance,const char* paramTypes,...);
	template<> jboolean Method::call<jboolean>(jobject instance,const char* paramTypes,...);
	template<> jint Method::call<jint>(jobject instance,const char* paramTypes,...);
	template<> jstring Method::call<jstring>(jobject instance, const char* paramTypes, ...);
	template<> jclass Method::call<jclass>(jobject instance, const char* paramTypes, ...);
	
	class Class {
		friend class Method;
		friend class Object;

		public:
			Class(const char* name,jclass clazz=NULL);
			~Class();

			inline jclass getClass() const {return mClass;}
			inline const char* getName() const {return mName.c_str();}
			Method* getMethod(const char* name,const char* signature=NULL);			
			Method* getStaticMethod(const char* name,const char* signature=NULL);
			Method* getConstructor(const char* signature=NULL);

			static Class* forName(const char* name);
			static Class* registerClass(jclass clazz);

		private:
			typedef core::CKHashMap<Method*> MethodMap;

			//JNIEnv* mJNI;
			jclass mClass;
			std::string mName;
			MethodMap mMethods;
	};

	class Object {
		public:
			/**
			* @param localRef true para indicar que se trata de una
			* referencia local, que será liberada automáticamente
			* una vez la instancia Object ha sido construida correctamente
			* la instancia. Debe ser true cuando se instancian objetos
			* que sólo se usarán dentro del ámbito del método nativo.
			*/
			Object(jobject instance,bool localRef);
			~Object();

			inline jobject getObject() {return mInstance;}

			Method* getMethod(const char* name,const char* signature=NULL);

			inline operator jobject() {return mInstance;}
			
		private:
			Class* mClass;
			jobject mInstance;
	};
}
