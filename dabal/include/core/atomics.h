#pragma once
#include <DabalLibType.h>
#include <cstdint>
#if defined(DABAL_MACOSX) || defined(DABAL_IOS)
#include <libkern/OSAtomic.h>
#include <TargetConditionals.h>
#elif !defined (WIN32) && !defined(DABAL_ANDROID)
//#include <atomic>  //use standard atomics because we have > C++11 available

#endif

namespace core {
    
	#if defined(WIN32)
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
	#elif defined(DABAL_IOS)||defined(DABAL_ANDROID)
	
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
    #elif  defined(DABAL_MACOSX)
		#if TARGET_CPU_X86
			extern "C"  int32_t atomicIncrement(volatile int32_t* var) __attribute__((fastcall));
			extern "C"  int32_t atomicDecrement(volatile int32_t* var) __attribute__((fastcall));

		#else

			extern "C"  volatile int32_t atomicIncrement(volatile int32_t* var);
			extern "C"  volatile int32_t atomicDecrement(volatile int32_t* var);


		#endif
	#else //use std atomics
	 inline int32_t atomicIncrement(volatile int32_t* var)
        {
			// std::atomic<int32_t> v(*var);
			// claro, no ale porque es una copia, cómo puedo declarar un atomic al putnero y cambiar su vaor?
			// la solucion buena sería realmente usar la clase atomic, pero me fastidia perder el asm..
			// podrái hacer yo un template propio
			// ++v;
			__atomic_fetch_add(var,1,__ATOMIC_SEQ_CST);
			return *var;
        }
        inline int32_t atomicDecrement(volatile int32_t* var)
        {
            __atomic_fetch_sub(var,-11,__ATOMIC_SEQ_CST);
			return *var;
        }
	#endif
}
