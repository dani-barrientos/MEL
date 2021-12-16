#ifdef _IOS
#include <TargetConditionals.h>
#endif

#if (TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR && defined(__arm__))

#include <core/Process.h>
using core::Process;
using core::MThreadAttributtes;

#include <core/ProcessScheduler.h>


#define mSwitchedOFF offsetof( MThreadAttributtes,mSwitched)
#define mIniSPOFF offsetof( MThreadAttributtes,mIniSP)
#define mRegistersOFF offsetof( MThreadAttributtes,mRegisters)
#define mCoRegistersOFF offsetof( MThreadAttributtes,mCoRegisters)
#define mLROFF offsetof( MThreadAttributtes,mLR)
#define mStackEndOFF offsetof( MThreadAttributtes,mStackEnd)
#define mStackSizeOFF offsetof( MThreadAttributtes,mStackSize)
#define mStackOFF offsetof( MThreadAttributtes,mStack)

volatile void Process::checkMicrothread( unsigned int msegs )
{
    volatile MThreadAttributtes* thisAux = this;

    asm volatile( "ldr r4,%[v]"::[v] "m" (thisAux):"r4","r0","r1" ); //tengo que engañarle aqui para que tenga en cuenta que se modifican, ya que se hace cuando se produce un cambio de contexto

    asm volatile( "str sp, [r4,%[v]]":: [v] "i" (mIniSPOFF) ); 

    asm volatile( "add r4,r4,%[v]":: [v] "i" (mRegistersOFF):"r4" );
    asm volatile( "stmia r4!,{r0-r12}");
    
    //ATENCION,IMPORTANTE. Para ser estrictos aqui habria que hacer unos ajustes que por la forma de compilar
    //en XCode no hace falta. Seguramente en algún momento haga falta en cuyo caso es hacer lo mismo que en la versión Android (no lo hago para no meter más incertidumbre ahora y pruebas y demas..)
	//TODO no me cuadra, r4 tiene que estar quedando un elemento (5 bytes menos). Por tanto apuntan do a mRegisters[12]--¿cómo funciona?
    asm volatile("vstmia r4,{d0-d15}");
    asm volatile( "sub r4,r4,%[v]":: [v] "i" (mCoRegistersOFF):"r4" );//restauramos posicion r4 al principio
    asm volatile( "str r4,[r4,%[v]]":: [v] "i" (mRegistersOFF+16) );//guar R4 en su posicion regitros + 4*4 --> MEJORAR
	
    asm volatile( "mov r5,#0\n" "add r5,pc,#68":::"r5" ); //TODO no se como obtener la direccion de un label!!
    //asm volatile( "ldr r5,3f" );
    asm volatile("str r5,[r4,%[v]]"::[v] "i" (mLROFF));
    asm volatile("ldrb r5, [r4,%[v]]"::[v] "i" (mSwitchedOFF));
    asm volatile("cmp r5,#0\n"
                 "beq 2f");
    
    asm volatile( "mov r5,#0" );
    asm volatile("strb r5,[r4,%[v]]"::[v] "i" (mSwitchedOFF) ); 

    asm volatile ("ldr r6,[r4,%[v]]"::[v] "i" (mStackEndOFF):"r6" );
    asm volatile ("ldr r5,[r4,%[v]]"::[v] "i" (mStackOFF) ); 
    asm volatile ("1: ldmdb r6!,{r8}":::"r8" ); 
    asm volatile ("stmfd sp!,{r8}");
    asm volatile ("cmp r6,r5\n"
                  "bne 1b" );
    asm volatile( "vldmia sp!,{d8-d15}");
    asm volatile ("ldmfd sp!,{r4-r11}" );
    asm volatile ("ldmfd sp!,{pc}\n" );
		 
    asm volatile("2:");
    
    execute( msegs );
    //asm volatile("execute_return:");
    //TODO no me convence este bloque. creo que solo debo necesitar los obligatorios
    asm volatile( "3:");
    asm volatile( "add r4,r4,%[v]":: [v] "i" (mRegistersOFF):"r4" );
    asm volatile( "ldmia r4,{r0-r3}":::"r0","r1","r2","r3");
    asm volatile( "add r4,r4,#20"); //apunto a R5
    asm volatile( "ldmia r4!,{r5-r12}");
    asm volatile("vldmia r4,{d0-d15};");
}

volatile void fakeFunction( ) __attribute__((noinline))  __attribute__( (used)) __attribute__((naked));
volatile void fakeFunction( )
{	
	asm volatile("wrapperSwitch:\n"
                 "stmfd sp!,{lr}" );
	asm volatile( "stmfd sp!, {r4-r11}" ); 
    asm volatile("vstmdb sp!,{d8-d15}");
	asm volatile ( "mov r1,#1");
	asm volatile ("strb r1, [r0,%[v]]":: [v] "i" (mSwitchedOFF) );
	asm volatile( "ldr r1,[r0,%[v]]":: [v] "i" (mIniSPOFF));
	asm volatile( "sub r1,r1,sp");

	asm volatile( "stmfd sp!,{r0}" );
	asm volatile( "bl _resizeStack" );
	asm volatile( "ldmfd sp!,{r0}"); 

	asm volatile ("ldr r2,[r0,%[v]]"::[v] "i" (mStackOFF) ); //inicio pila
	asm volatile ("ldr r3,[r0,%[v]]\n"::[v] "i" (mIniSPOFF) );
	asm volatile(
				"4:\n"
				 "ldmia sp!,{r1}\n");
	asm volatile("stmia r2!,{r1}\n");
	asm volatile("cmp sp,r3\n");
	asm volatile("bne 4b\n");

	asm volatile( "ldr r4,[r0,%[v]]":: [v] "i" (mRegistersOFF+16) ); //16 = 4*4
	asm volatile( "ldr pc,[r0,%[v]]"::[v] "i" (mLROFF) );
}

void Process::_switchProcess( )
{
	Process* p = ProcessScheduler::getCurrentProcess();
    MThreadAttributtes* mt = p;


    asm volatile( "ldr r0,%0\n"::"m" (mt):"r0");
    asm volatile( "bl wrapperSwitch" );
}

#endif

