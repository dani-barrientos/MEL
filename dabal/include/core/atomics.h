#pragma once
#include <FoundationLibType.h>
#include <cstdint>
#ifdef _ANDROID
#include <stdatomic.h>
#elif defined(_MACOSX) || defined(_IOS)
#include <libkern/OSAtomic.h>
#include <TargetConditionals.h>
#elif defined(_ANDROID)
#include <sys/atomics.h>
#endif

namespace core {
    
	#ifdef _WIN64
	inline int32_t __fastcall atomicIncrement(volatile int32_t* var) {
		//!@TODO 
		return ++(*var);
	}
	inline int32_t __fastcall atomicDecrement(volatile int32_t* var) {
		//@TODO!!!
		return --(*var);
	}
	#elif defined(WIN32)
	//fastcall and inline seems to not be managed by compiler,just in case..
	inline int32_t __fastcall atomicIncrement(volatile int32_t* var) {
         _asm {

			mov eax,1
			lock xadd dword ptr [ecx],eax; //__fastcall implies parameter is passed in register
			inc eax;
			
		};

	}
	inline int32_t __fastcall atomicDecrement(volatile int32_t* var) {

     _asm {
			//mov		ecx,var
			//lock dec dword ptr [ecx]
			mov eax,-1
			lock xadd dword ptr [ecx],eax; //__fastcall implies parameter ys passed in register
			dec eax;
		};

	}
	#elif defined(_IOS)||defined(_ANDROID)
    #if !TARGET_IPHONE_SIMULATOR
    volatile int32_t atomicIncrement(volatile int32_t* var)  __attribute__((naked));
    volatile int32_t atomicDecrement(volatile int32_t* var)  __attribute__((naked));
    #else //target simulator
        //el codigo es igual que para MacOS pero no me preocupo
		//y lo dejo con llamada al SO por si acaso
        inline int32_t atomicIncrement(volatile int32_t* var)
        {
            return OSAtomicIncrement32( (int*)var );
        }
        inline int32_t atomicDecrement(volatile int32_t* var)
        {
            return OSAtomicDecrement32( (int*)var );
        }
    #endif
    #elif  defined(_MACOSX)
    #if TARGET_CPU_X86
        extern "C"  int32_t atomicIncrement(volatile int32_t* var) __attribute__((fastcall));
        extern "C"  int32_t atomicDecrement(volatile int32_t* var) __attribute__((fastcall));

    #else

        extern "C"  volatile int32_t atomicIncrement(volatile int32_t* var);
        extern "C"  volatile int32_t atomicDecrement(volatile int32_t* var);


    #endif

#endif
}
