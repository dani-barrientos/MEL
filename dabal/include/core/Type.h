#pragma once

#include <DabalLibType.h>
#include <string>
#include <map>

#include <core/CriticalSection.h>

/**
 * Utility macro for easily declaring runtime type info attributes and methods.
 * Just place this macro right after the class declaration it will automatically
 * define any attributes and methods for making the class "runtime type-aware".
 */
#define DABAL_CORE_OBJECT_TYPEINFO_ROOT \
	private: \
	static const ::core::Type TYPE; \
	public: \
	inline static const ::core::Type &type(){return TYPE;} \
	virtual const ::core::Type &getMyType() const {return TYPE;} \
	private:

#define DABAL_CORE_OBJECT_TYPEINFO \
	private: \
	static const ::core::Type TYPE; \
	public: \
	inline static const ::core::Type &type(){return TYPE;} \
	virtual const ::core::Type &getMyType() const override {return TYPE;} \
	private:


/**
* Utility macro for easily implementing runtime type info methods for root classes.
* Just place this macro anywhere in the cpp file of the target class 
* and it will automatically define the method bodies for the runtime type info
* methods.
* @param _type_ the type of the class you want to implement the runtime type methods. It 
* MUST be a valid class name (i.e. a name that can be resolved by the C++ compiler)
*/
#define DABAL_CORE_OBJECT_TYPEINFO_IMPL_ROOT(_type_) \
	const ::core::Type _type_::TYPE(#_type_); 

/**
* Utility macro for easily implementing runtime type info methods for derived classes.
* Just place this macro anywhere in the cpp file of the target class 
* and it will automatically define the method bodies for the runtime type info
* methods.
* @param _type_ the type of the class you want to implement the runtime type methods. It 
* MUST be a valid class name (i.e. a name that can be resolved by the C++ compiler)
* @param _ancestorType_ the type the super class. It MUST be a valid class name too.
*/
#define DABAL_CORE_OBJECT_TYPEINFO_IMPL(_type_,_ancestorType_) \
	const ::core::Type _type_::TYPE(#_type_,&_ancestorType_::type());

#define DABAL_CORE_OBJECT_TYPEINFO_IMPL2(_type_,_ancestorType1_,_ancestorType2_) \
	const ::core::Type _type_::TYPE(#_type_,&_ancestorType1_::type(),&_ancestorType2_::type() );

#define DABAL_CORE_OBJECT_TYPEINFO_IMPL3(_type_,_ancestorType1_,_ancestorType2_,_ancestorType3_) \
	const ::core::Type _type_::TYPE(#_type_,&_ancestorType1_::type(),&_ancestorType2_::type(),&_ancestorType3_::type() );

#define DABAL_CORE_OBJECT_TYPEINFO_IMPL4(_type_,_ancestorType1_,_ancestorType2_,_ancestorType3_,_ancestorType4_) \
	const ::core::Type _type_::TYPE(#_type_,&_ancestorType1_::type(),&_ancestorType2_::type(),&_ancestorType3_::type(),&_ancestorType4_::type() );


/**
* @namespace core
* @brief core system classes and functions
*/
namespace core {
	/**
	 * Tag class representing an "object type".
	 * It's used throughout the Judas engine for managing runtime generic data types 
	 * without having to enable C++ runtime-type information.<br>
	 * It allows up to four base clases (multiple inheritance)
	 */

	class DABAL_API Type {
		private:

			class DABAL_API ReflectionMap
			{
				friend class Type;
			public:


				ReflectionMap();
				~ReflectionMap();
				const Type * operator[](const char * name);

			private:

				CriticalSection mCS;
				std::map<std::string, const Type *> mReflectionMap;

				void registerType (const char * name, const Type * type);
				void unregisterType (const char * name);
			};

		public:
			
			inline static ReflectionMap& Reflection()
			{
				// En principio no es necesario hacer thread-safe este m�todo para evitar la construcci�n duplicada
				// del ReflectionMap, porque no existe posibidad ya que se crear� en la fase de inicializaci�n, siendo
				// todas las llamadas desde el mismo hilo de aplicaci�n.
				static ReflectionMap mReflection;
				return mReflection;
			}

		private:

			const char* mName;
			const unsigned char mNumParents;
			const Type* mParent[4];

		protected:
		public:

			static const Type NOTYPE;

			/**
			 * Creates a new derived type.
			 * Used when creating new types representing objects inheriting from
			 * already existing types.
			 * @param name the type name. You usually pass the actual C++ class name, but
			 * any string value is valid.
			 * @param super the "super type" this new one is descending from
			 */
			Type(const char* name,const Type *super);
			Type(const char* name,const Type* parent1,const Type* parent2);
			Type(const char* name,const Type* parent1,const Type* parent2, const Type* parent3);
			Type(const char* name,const Type* parent1,const Type* parent2, const Type* parent3,const Type* parent4);
			/* Creates a new root type.
			 * Used when creating a new super type with no ancestors.
			 * @param name the type name
			 */
			Type(const char* name);
			virtual ~Type();
			
			/**
			 * Get the type name
			 * @return the type name this type represents :P
			 */
			inline const char* getName() const;
			/**
			 * Equality operator
			 * Checks if the given type is equal to this one.
			 * @param t the type to be checked for equality
			 * @return true if both types are considered "equal". false otherwise
			 */
			inline bool operator ==(const Type &t) const;
			inline bool operator !=(const Type& t) const;
			/**
			 * Inheritance check
			 * Used when checking for valid inheritance against types.
			 * @param t the super type to check inheritance against
			 * @return true if this instance can be considered a subtype of the given type
			 * @remarks due to multiple inheritance, this function has some cost, try to not use it very much
			 */
			bool instanceOf(const Type &t) const;
			/**
			 * Get super type
			 * @return a pointer to the super type (NULL if this instance is root type)
			 */
			inline const Type* getSuper( unsigned char i = 0) const;
			inline unsigned char getNumParents() const{ return mNumParents;}
	};

	//inline
	const Type* Type::getSuper( unsigned char i) const 
	{
		return mParent[i];
	}
	const char* Type::getName() const {
		return mName;
	}
	bool Type::operator ==(const Type &t) const {
		return &t==this;
	}
	bool Type::operator !=(const Type& t) const {
		return &t!=this;
	}
}
