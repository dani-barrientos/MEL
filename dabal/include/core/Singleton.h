///////////////////////////////////////////////////////////
//  Singleton.h
//  Implementation of the Class Singleton
//  Created on:      29-mar-2005 10:00:13
// CLASS FOR SINGLETONS THAT ARE ABSTRACT CLASSES
// YOU MUST IMPLEMENT A CREATION FUNCITON: See createSingleton at this file.
//
// TODO no está bien del todo el lock/unlock en los accessos. REVISAR DETENIDAMENTE
// You can use Singleton with a SmartPtr. See template parameters
///////////////////////////////////////////////////////////

#pragma once

#include <FoundationLibType.h>
#include <core/SmartPtr.h>
#include <mpl/Type2Type.h>
using mpl::Type2Type;
#include <mpl/_If.h>
using mpl::_if;

#include <core/Exception.h>
using core::Exception;

#include <core/Singleton_Singlethread_Policy.h>
using core::Singleton_Singlethread_Policy;
namespace core
{
	/**
	* @class Singleton
	 * template class for singleton classes use: class A : public Singleton<A>
	 * Singletons can be Abstract or concrete. In abstract singleton (default behaviour), you provide your own creation funcion
	 * son Singletong can be inherited.Although, abstract singleton is neccesary when you need to customize singleton
	 * creation (passing arguments to constructor). If singleton is declared as non-abstract, default constructor is
	 * called on first getSingleton call.
	 * Singleton must be deleted when no more needed using deleteSingleton function. When deleteSingleton es used
	 * it check for No more references to object (if using SmartPtr, of course) and throw an Exception if there are more references.
	 * If you don't like this behaviour you must overwrite deleteSingleton and do your own work.
	 * You can use SmartPtr for internal singleton. For that purpose use third parameter
	 * and put true. Example:
	 *    class A: public Singleton<A,isAbstract,true>
	 * In that case, you can safety use SmartPtr's to singletons, but when you do deleteSingleton
	 * all other references to object should be removed.
	 * @todo probablemente debería pasar el tipo de puntero, pero así lo facilito un poco
	 */

	///@cond HIDDEN_SYMBOLS
    template <class T,bool useSmartPtr, template <class> class ThreadingPolicy> class Singleton_Base
    {
        public:
           static bool isSingletonCreated()
            {
                return mObject != 0;
            }
        protected:
            typedef typename _if< useSmartPtr, SmartPtr<T> ,T*>::Result Tpointer;
            //typedef typename _if< Conversion< T,IRefCount*>::Exists, SmartPtr<T> ,T*>::Result Tpointer;
			typedef typename ThreadingPolicy<Tpointer>::VolatileType  InstanceType;
            static InstanceType mObject; //TODO considerar el VolatileType, importante
	
			//@warning ignominia absoluta para poder establecer el mObject desde una clase hija
			//ya que no me deja con dllexport
			//solo puede llamarse una vez por singleton
			static void setObject( T* ob )
			{
				mObject = ob;
			}

    };

	template <class T,bool> struct GetObjectPtr
	{
		static T* get(T* p){ return p;}
	};
	//specialization for SmartPtr
	template <class T> struct GetObjectPtr<T,true>
	{
		static T* get( SmartPtr<T>& p){ return p.getPtr();}
	};

    template <class T,bool,bool,template<class> class> class Singleton; //predeclaration
    //version for non-abstract singletons
    template <class T,template<class> class ThreadingPolicy,bool useSmartPtr,bool isAbstract>
    class Singleton_Middle : public Singleton_Base<T,useSmartPtr,ThreadingPolicy>  //¡nombre cutre!!
    {
        public:
			typedef ThreadingPolicy<T> TThreadingPolicy;
            inline static T& getSingleton()
            {
                return *getSingletonPtr();
            }
            inline static T* getSingletonPtr()
            {
				typename ThreadingPolicy<T>::Lock block;
                if ( Singleton_Base<T,useSmartPtr,ThreadingPolicy>::mObject == (T*)NULL )
                {
                    Singleton_Base<T,useSmartPtr,ThreadingPolicy>::mObject = Singleton<T,isAbstract,useSmartPtr,ThreadingPolicy>::createObject( Type2Type<T>());
                }
                return GetObjectPtr<T,useSmartPtr>::get(Singleton_Base<T,useSmartPtr,ThreadingPolicy>::mObject); 
            }
    };
    //specialization for abstract
    template <class T,template<class> class ThreadingPolicy,bool useSmartPtr>
    class Singleton_Middle<T,ThreadingPolicy,useSmartPtr,true> : public Singleton_Base<T,useSmartPtr,ThreadingPolicy>  //¡nombre cutre!!
    {
        public:
            inline static T& getSingleton()
            {
				typename ThreadingPolicy<T>::Lock block;
                return *Singleton_Base<T,useSmartPtr,ThreadingPolicy>::mObject;
            }
            inline static T* getSingletonPtr()
            {
				typename ThreadingPolicy<T>::Lock block;
				return GetObjectPtr<T,useSmartPtr>::get(Singleton_Base<T,useSmartPtr,ThreadingPolicy>::mObject); 
                //return Singleton_Base<T,useSmartPtr,ThreadingPolicy>::mObject;
            }

    };
	///@endcond
	template<class T, bool isAbstract = true,bool useSmartPtr = true,
		template<class> class ThreadingPolicy = ::core::Singleton_Singlethread_Policy >
	class Singleton : public Singleton_Middle<T,ThreadingPolicy,useSmartPtr,isAbstract>
	{
	    friend class Singleton_Middle<T,ThreadingPolicy,useSmartPtr,isAbstract>;
	public:


		static void deleteSingleton()
		{
			typename ThreadingPolicy<T>::Lock block;
			ObjectDeleter<T,useSmartPtr>::doDelete( &Singleton_Base<T,useSmartPtr,ThreadingPolicy>::mObject );
		}

		/**
		 * Singleton creation. it must be called before use it. This function must be
		 * reimplemented in abstract class's child
		 * NO FUNCIONA EN dllexport
		 */
		// static void createSingleton();




    private:

       /**
        * tricks for friend relationship
        **/
        template <class U>
        static U* createObject( Type2Type<U> )
        {
           return new U();
        }
        static void deleteObject(T* obj)
        {
            delete obj;
        }


        template <class U,bool> class ObjectDeleter
        {
            //deleter for plain pointer
            public:
                static void doDelete( U** obj )
                {
                     Singleton<T,isAbstract,useSmartPtr,ThreadingPolicy>::deleteObject( *obj );
					 *obj = NULL;

                }
        };

        template <class U> class ObjectDeleter<U,true>
        {
            //deleter for SmartPtr
            public:
                static void doDelete( SmartPtr<U>* obj)
                {
                    //throw exception if obj has other references
                    if ( obj->getPtr() && (*obj)->getRefCount() > 1 )
                    {
                        throw Exception( "Singleton::deleteSingleton. Other references to obj still alive" );
                    }
                    *obj = 0;
                }
        };


	};
	///@cond
	template <class T,bool useSmartPtr,template <class > class ThreadingPolicy> typename Singleton_Base<T,useSmartPtr,ThreadingPolicy>::InstanceType Singleton_Base<T,useSmartPtr,ThreadingPolicy>::mObject = 0;
	///@endcond
//default version
//normally each child will need his own version

/*//con dllexport no me vale esto!!
template <class T >  void Singleton<T>::createSingleton()
{
	if ( !mObject  )
	{
		mObject = new T();
	}
}*/


}

