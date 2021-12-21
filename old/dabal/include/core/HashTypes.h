#pragma once

#include <assert.h>
#include <string>
//TODO: use a hash map!!!
#include <map>

#include <DabalLibType.h>
#include <core/SmartPtr.h>
namespace core {
	using ::core::SmartPtr;
	using std::string;
	using std::map;
	using std::pair;

	/**
	 * Utility struct to be used in char-based hash keys
	 */
	struct less_str {
		/**
		 * Strict weak comparison operator for strings.
		 * @param x string to compare
		 * @param y string to compare
		 * @return true if x is "less" than y. false otherwise
		 */
		bool operator()(const char* x, const char* y) const {				
			if ((strcmp(x, y) < 0))
				return true;
			return false;
		}
	};

	/**
	 * Base template class for building string-keyed maps.
	 * <p>Provides basic support for handling "removed" elements and the ability to
	 * provide a "resolver" method for auto-generating new elements if they do not
	 * yet exist at request-time.</p>
	 * @tparam T the type of the elements in the map (the keys will always be strings)
	 */
	template <class T> class CKHashMapBase:public map<const char*,T,less_str> {
		public:
            /** Convenient alias for CKhashMap value type */
            typedef typename map<const char*,T>::value_type Entry;
        
			/**
			 * Creates a new string-key map.
			 *
			 * <p>When creating a new map with the handleLost flag set to true, all elements
			 * will be handled by CKHashMapBase::handleLost method before they are removed.</p>
			 * <p>Element removal can happen direcly, like when CKHashMap::remove or CKHashMap::clear
			 * are called, or indirectly, like when calling CKHashMap::put for ovewriting existing elements.</p>
			 * <p>Special care must be taken when setting the handleLost flag to true: the
			 * map MUST be empty by the time it's destroyed, for CKHashMapBase::handleLost is virtual and
			 * thus cannot be called from within the destructor code.</p>
			 *
			 * @param doHandleLost true to indicate the elements in the map need to
			 * be "handled" before they are removed from the map. false otherwise (meaning
			 * removed elements will be just discarded without taking any further action)
			 * @param def the default value to return when retrieving non-existing keys
			 *
			 * @see handleLost
			 */
			CKHashMapBase(const bool doHandleLost,T def):mHandleLost(doHandleLost),mDefault(def) {}
			CKHashMapBase(const CKHashMapBase<T>& m) {
				*this = m;
			}

			/**
			 * Standard destructor.
			 * If the map was created with the handleLost flag set to true, an assertion
			 * is raised if the map is not yet empty, as there is no way for calling 
			 * the virtual method CKHashMapBase::handleLost once in the destructor.
			 */
			virtual ~CKHashMapBase() {
				//NOTE: clear must not be called, as it invokes handleLost,
				//which wouldn't be a valid call when issued from within
				//the destructor
				assert(!mHandleLost || this->size()==0);
				typename CKHashMapBase<T>::iterator ki;
				while (this->size()>0) {
					ki=this->begin();
					const char* key=ki->first;
					this->erase(ki);
					delete []key;
				}
			}

			/**
			 * Element resolver.
			 * <p>Creates a new element on the map under the given key.</p>
			 * <p>This method is called whenever a call to CKHashMapBase::get or
			 * CKHashMapBase::operator[] is made and there is no element for the requested
			 * key.</p>
			 * <p>When this happens, this method is called so the new element is added to
			 * the map automatically and returned as the result of the initial query call
			 * seamlessly.</p>
			 * <p>At this base level, there is no actual element creation. It just
			 * returns the default value specified upon map construction.
			 * Subclasses must provide it's own implementatin if they wish to
			 * add support for creating new elements at request-time.</p>
			 * @param name the key of the new element to be created
			 * @return a valid instance element
			 */
			virtual T resolve(const char* name) const {
				return mDefault;
			}
			/**
			 * Element removal handler.
			 * <p>Gets called whenever the map needs to remove an element, either as 
			 * a result of direct or indirect removal call.</p>
			 * <p>Subclasses must provide the concrete implementation if they wish to
			 * take any special care of elements removed (i.e. deleting pointers, 
			 * free aditional resources, etc.)</p>
			 */
			virtual void handleLost(T t) const=0;

			/**
			 * Clears the map.
			 * <p>If the map was created with the handleLost flag set, then all elements
			 * removed are processed by CKHashMapBase::handleLost before they are finally
			 * discarded.</p>
			 */
			void clear() {					
				typename CKHashMapBase<T>::iterator ki;
				while (this->size()>0) {
					ki=this->begin();
					const char* key=ki->first;
					if (mHandleLost)
						handleLost(ki->second);
					this->erase(ki);
					delete []key;
				}
			}
			/**
			* override for map::erase to delete key
			*/
			/*
			iterator erase( iterator i)
			{
				iterator result;
				const char* key=i->first;
				delete []key;
				result = map<const char*,T,less_str>::erase( i );
				return result;
			}*/


			/**
			 * Retrieves an element from the map.
			 *
			 * @param name they key of the element to be retrieved
			 * @return the requested element if the key is valid. If there was no such 
			 * a key in the map, then the default value specified upon map creation is returned.
			 *
			 * @see operator[]
			 */
			virtual T get(const char* name) {
				typename CKHashMapBase<T>::iterator i=this->find(name);
				return i!=this->end()?i->second:mDefault;
			}

			/**
			 * Check for key.
			 *
			 * @param name the name of the key to be checked
			 * @return true if the key exists in the map. false otherwise
			 */
			bool containsKey(const char* name) const {
				typename CKHashMapBase<T>::const_iterator i=this->find(name);
				return i!=this->end();
			}

			/**
			 * Add a new element into the map.
			 *
			 * <p>If the given key already existed, CKHashMapBase::handleLost will be called 
			 * on the old value (if the map was created with the handleLost flag set), and 
			 * then it will be overwrite with the new one.</p>
			 *
			 * @param name the key under with the element will be stored
			 * @param value the element to be added
			 * @return true if the key already existed and was replaced.
			 * false otherwise (key didn't exist and has just been created)
			 *
			 * @see get, operator[]
			 */
			bool put(const string& name,T value) {
				return put(name.c_str(),value);
			}

			/**
			* Add a new element into the map.
			*
			* <p>If the given key already existed, CKHashMapBase::handleLost will be called 
			* on the old value (if the map was created with the handleLost flag set), and 
			* then it will be overwrite with the new one.</p>
			*
			* @param name the key under with the element will be stored.
			* @param value the element to be added
			* @return true if the key already existed and was replaced.
			* false otherwise (key didn't exist and has just been created)
			*
			* @see get, operator[]
			*/
			bool put(const char* name,T value) {
				typename CKHashMapBase<T>::iterator i=this->find(name);
				unsigned int len=(unsigned int)strlen(name)+1;
				char* ch=NULL;
                bool existed=(i!=this->end());

				if (existed) {
					if (mHandleLost) handleLost(i->second);
					if (strlen(i->first)>=len-1)
						ch=(char *)i->first;
					this->erase(i);
				}
				if (!ch) {
					ch=new char[len];
				}
				strcpy(ch,name);
				this->insert(pair<const char*,T>(ch,value));
				return existed;
			}

			/**
			* Remove an element from the map.
			* <p>If the key exists and the map was created with the handleLost flag set,
			* then CKHashMapBase::handleLost will be invoked right before the element
			* if removed.</p>
			*
			* @param name the key of the element to be removed
			* @return true if the key existed and the element was removed.
			* false otherwise.
			*/
			bool remove(const char *name) {
				typename CKHashMapBase<T>::iterator i=this->find(name);
				if (i!=this->end()) {
					if (mHandleLost) handleLost(i->second);
					delete []i->first;
					this->erase(i);
					return true;
				}
				else
					return false;
			}

			/**
			 * Retrieves an element from the map.
			 *
			 * <p>If the requested key does not yet exist, then CKHashMapBase::resolve is
			 * called and the result is automatically added to the map and finally returned
			 * to the caller.</p>
			 *
			 * @param name the element key to be retrieved.
			 * @return the requested value.
			 */
			T operator[](const string& name) {
				return operator[](name.c_str());
			}

			/**
			* Retrieves an element from the map.
			*
			* <p>If the requested key does not yet exist, then CKHashMapBase::resolve is
			* called and the result is automatically added to the map and finally returned
			* to the caller.</p>
			*
			* @param name the element key to be retrieved.
			* @return the requested value.
			*/
			T operator[](const char* name) {
				typename CKHashMapBase<T>::const_iterator i=this->find(name);
				if (i!=this->end())
					return i->second;
				else {
					int len=(int)(strlen(name)+1);
					char* ch=new char[len];
					strcpy(ch,name);
					T t=resolve(name);
					this->insert(pair<const char*,T>(ch,t));
					return t;
				}						
			}

			/**
			* Assignment operator
			* Resets the hash map so it mimics the contents of the given one.
			* All atrributes are COPIED, including the "handle lost" flag and
			* the default value.
			*
			* The contents of the map itself are also COPIED over from the source map
			* (beware when copying maps containing pointers!).
			*
			* The keys are always recreated on the current map, as the contents are
			* simply inserted via a standard put operation.
			*
			* @param m the source hash map to assign this map from
			*/
			CKHashMapBase<T>& operator=(const CKHashMapBase<T>& m) {
				clear();
				mHandleLost = m.mHandleLost;
				mDefault = m.mDefault;
				for (auto& e : m) {
					put(e.first, e.second);
				}
				return *this;
			}

		protected:
			bool mHandleLost;
			T mDefault;
	};
	/**
	* metafunction to delete pointer
	*/
	template<class T> struct PointerDeleter
	{
		static void _delete( T p )
		{
			delete p;
		}
	};
	//specialization for SmartPtr
	template<class T> struct PointerDeleter< SmartPtr<T> >
	{
		static void _delete( SmartPtr<T>& p )
		{
			//do nothing
		}
	};
	/**
	 * Specialized map for storing pointer-like values.
	 * <p>Provides implementation for the virtual method CKHashMapBase::handleLost.</p>
	 * @tparam T the type of the elements in the map, that MUST be a valid pointer-like type
	 * (either a regular pointer, a smart Reference or any type the compiler can resolve
	 * as a pointer).
	 */
	template <class T> class CKHashMap:public CKHashMapBase<T> {
		public:
			/**
			 * Creates a new pointer-value map.
			 * @param doHandleLost flag indicating if elements should be processed before
			 * they are removed from the map (that is, whether should they be deleted)
			 * @param def the default value to return when retrieving non-existing keys
			 * (defaults to NULL)
			 */
			CKHashMap(const bool doHandleLost,const T def=NULL):CKHashMapBase<T>(doHandleLost,def) {}

			/**
			 * Takes care of a removed element.
			 * <p>In this implementation, it just deletes the element (that is supposed
			 * to be a pointer-like value).</p>
			 * @param t the element to be removed
			 */
			virtual void handleLost(T t) const 
			{
			//	delete t;
				PointerDeleter<T>::_delete( t );
			}
	};

	/**
	* Specialized map for storing plain values.
	* <p>Provides implementation for the virtual method CKHashMapBase::handleLost.</p>
	* @tparam T the type of the elements in the map.
	*/
	template <class T> class CKHashMapValue:public CKHashMapBase<T> {
		public:
			/**
			* Creates a new map.
			* @param doHandleLost flag indicating if elements should be processed before
			* they are removed from the map. This has no effect at this implementation level,
			* since CKHashMapValue::handeLost implementation does nothing.
			* @param def the default value to return when retrieving non-existing keys.
			*/
			CKHashMapValue(const bool doHandleLost,const T def):CKHashMapBase<T>(doHandleLost,def) {}

			/**
			 * Processes an element before removing it from the map.
			 * <p>Current implementation does nothing. Subclasses will need to override
			 * this method if they wish to apply some additional processing.</p>
			 * @param t the element being removed
			 */
			virtual void handleLost(T t) const {}
	};
}

