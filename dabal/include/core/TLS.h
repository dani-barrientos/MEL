#pragma once
#ifdef _WINDOWS
#include <Windows.h>
#elif defined (_MACOSX) || defined(_IOS) || defined(_ANDROID)
#include <pthread.h>
#endif
#include <DabalLibType.h>
#include <mpl/TypeTraits.h>

namespace core
{
	/**
	* @class TLS
	* @brief Thread Local Storage
	* @todo EN OBRAS
	*/
	class DABAL_API TLS
	{
	public:
#ifdef _WINDOWS 
	typedef DWORD TLSKey;
#elif defined (_MACOSX) || defined(_IOS) || defined(_ANDROID)
	typedef pthread_key_t TLSKey;
#elif defined(_AIRPLAY)
	typedef int TLSKey; //TODO
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
