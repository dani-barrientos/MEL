#include <core/TLS.h>
using core::TLS;
#ifdef _AIRPLAY
#include <map>
using std::map;

static TLS::TLSKey gCurrentKey = 0; //TODO para salri del paso en AIRPLAY!!
static map< TLS::TLSKey,const void* > gKeysMap;
#endif
bool TLS::createKey(TLSKey& key ) {
#ifdef _WINDOWS
	DWORD result = TlsAlloc();
	key = result;
	return (result!=TLS_OUT_OF_INDEXES);

#elif defined(DABAL_LINUX) || defined(DABAL_ANDROID) || defined(DABAL_IOS) || defined (DABAL_MACOSX)
	int ok=pthread_key_create(&key,NULL);
	return ok==0;
#endif
}

bool TLS::deleteKey(mpl::TypeTraits<TLSKey>::ParameterType key) {
#ifdef _WINDOWS
	BOOL ok= TlsFree(key);
	return ok!=0;

#elif defined(DABAL_LINUX) || defined(DABAL_ANDROID) || defined(DABAL_IOS) || defined (DABAL_MACOSX)
	int ok=pthread_key_delete(key);
	return ok==0;
#endif
}

bool TLS::setValue(mpl::TypeTraits<TLSKey>::ParameterType key, const void* value ) {
#ifdef _WINDOWS
	BOOL ok=TlsSetValue( key, (LPVOID)value );
	return ok!=0;

#elif defined(DABAL_LINUX) || defined(DABAL_ANDROID) || defined(DABAL_IOS) || defined (DABAL_MACOSX)
	int ok=pthread_setspecific(key,value);
	return ok==0;
#endif
}

void* TLS::getValue(mpl::TypeTraits<TLSKey>::ParameterType key) {
#ifdef _WINDOWS
	return TlsGetValue( key );

#elif defined(DABAL_LINUX) || defined(DABAL_ANDROID) || defined(DABAL_IOS) || defined (DABAL_MACOSX)
	return pthread_getspecific(key);
#endif
}
