#pragma once
#ifdef _WINDOWS
#include <Windows.h>
#else
#include <pthread.h>
#endif
#include <DabalLibType.h>
#include <mpl/TypeTraits.h>
namespace mel
{
	namespace core
	{
		/**
		* @class TLS
		* @brief Thread Local Storage
		* @todo EN OBRAS
		*/
		class MEL_API TLS
		{
		public:
	#ifdef _WINDOWS 
		typedef DWORD TLSKey;
	#elif defined(MEL_LINUX) || defined (MEL_MACOSX) || defined(MEL_ANDROID) || defined(MEL_IOS)
		typedef pthread_key_t TLSKey;
	#endif

		/**
		* create new Key in TLS for all threads. Each threads will bind a different value for this key
		* @param[out] key New key create
		* @return true if the key was created successfully. false otherwise
		* @remarks usually key is created at application begin and will be save in a static variable
		*/
		static bool createKey(TLSKey& key );
		/**
		* @param[in] key the key to be deleted
		* @return `true` if the key was successfully deleted; `false` otherwise
		*/
		static bool deleteKey(mpl::TypeTraits<TLSKey>::ParameterType key);
		/*
		* @param key the key to set tehe value for
		* @param value the value to be set
		* @return true if the value was successfully set. false otherwise
		*/
		static bool setValue(mpl::TypeTraits<TLSKey>::ParameterType key, const void* value );
		/**
		* @param key the key to get the value from
		* @return the value of the key (or NULL if the value retrieval falied, or
		* the key is not valid)
		*/
		static void* getValue(mpl::TypeTraits<TLSKey>::ParameterType key );
		};
	};
}