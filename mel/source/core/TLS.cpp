#include <core/TLS.h>
using mel::core::TLS;
bool TLS::createKey(TLSKey& key ) {
#ifdef _WINDOWS
	DWORD result = TlsAlloc();
	key = result;
	return (result!=TLS_OUT_OF_INDEXES);

#elif defined(MEL_LINUX) || defined(MEL_ANDROID) || defined(MEL_IOS) || defined (MEL_MACOSX)
	int ok=pthread_key_create(&key,NULL);
	return ok==0;
#endif
}

bool TLS::deleteKey(mpl::TypeTraits<TLSKey>::ParameterType key) {
#ifdef _WINDOWS
	BOOL ok= TlsFree(key);
	return ok!=0;

#elif defined(MEL_LINUX) || defined(MEL_ANDROID) || defined(MEL_IOS) || defined (MEL_MACOSX)
	int ok=pthread_key_delete(key);
	return ok==0;
#endif
}

bool TLS::setValue(mpl::TypeTraits<TLSKey>::ParameterType key, const void* value ) {
#ifdef _WINDOWS
	BOOL ok=TlsSetValue( key, (LPVOID)value );
	return ok!=0;

#elif defined(MEL_LINUX) || defined(MEL_ANDROID) || defined(MEL_IOS) || defined (MEL_MACOSX)
	int ok=pthread_setspecific(key,value);
	return ok==0;
#endif
}

void* TLS::getValue(mpl::TypeTraits<TLSKey>::ParameterType key) {
#ifdef _WINDOWS
	return TlsGetValue( key );

#elif defined(MEL_LINUX) || defined(MEL_ANDROID) || defined(MEL_IOS) || defined (MEL_MACOSX)
	return pthread_getspecific(key);
#endif
}
