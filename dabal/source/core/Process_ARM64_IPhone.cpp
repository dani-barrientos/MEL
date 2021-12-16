/**
 * now this file is commen for new (at 2020) ARM64 architecture for Mac
 */
/*#if defined(_IOS) || defined(TARGET_CPU_ARM64)

#include <TargetConditionals.h>
#endif
*/
#if (/*TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR && */defined(__arm64__) )
#include <TargetConditionals.h>
#include <core/Process.h>
using core::Process;
using core::MThreadAttributtes;

#include <core/ProcessScheduler.h>


#define mSwitchedOFF offsetof( MThreadAttributtes,mSwitched)
#define mIniSPOFF offsetof( MThreadAttributtes,mIniSP)
#define mRegistersOFF offsetof( MThreadAttributtes,mRegisters)
#define mVRegistersOFF offsetof( MThreadAttributtes,mVRegisters)
#define mLROFF offsetof( MThreadAttributtes,mLR)
#define mStackEndOFF offsetof( MThreadAttributtes,mStackEnd)
#define mStackOFF offsetof( MThreadAttributtes,mStack)
/**
 * notas:
  r0-r7 parametros y variables locales
  r19-r29 deben preservarse (
  r8 - algo de retorno (referencia?)
  r9-r15: temporales
  r16,r17,r18- dependiente plataforma
 */

volatile void Process::checkMicrothread( unsigned int msegs )
{
    volatile MThreadAttributtes* thisAux = this;
    void* ptr = &&continueexecuting; //direccion absoluta de etiqueta.
    asm volatile( "ldr x10,%[v]"::[v] "m" (thisAux):"x10","x0","x1" ); //tengo que engañarle aqui para que tenga en cuenta que se modifican, ya que se hace cuando se produce un cambio de contexto (y desde su punto de vista no hay nada de eso, por lo que el compilador podría no salvagaurdar estos y por tanto error al usarlos al retornar)
    //store return label address in local x11. Can be calcualted directly as a constant because address calculation if more complicated, involving adr instruction
    asm volatile( "ldr x11,%[v]"::[v] "m" (ptr):"x11");
    asm volatile( "mov x9,sp":::"x9"); //sp to auxiliar register
    asm volatile( "str x9, [x10,%[v]]":: [v] "i" (mIniSPOFF)); //save sp

    asm volatile("stp x19,x20,[x10,%[v]]":: [v] "i" (mRegistersOFF));
    asm volatile("stp x21,x22,[x10,%[v]]":: [v] "i" (mRegistersOFF+16));
    asm volatile("stp x23,x24,[x10,%[v]]":: [v] "i" (mRegistersOFF+32));
    asm volatile("stp x25,x26,[x10,%[v]]":: [v] "i" (mRegistersOFF+48));
    asm volatile("stp x27,x28,[x10,%[v]]":: [v] "i" (mRegistersOFF+64));
    asm volatile("str x29,[x10,%[v]]":: [v] "i" (mRegistersOFF+80));

    //save SIMD registers. D = bottom 64 bit of v registers
    asm volatile("stp d8,d9,[x10,%[v]]":: [v] "i" (mVRegistersOFF));
    asm volatile("stp d10,d11,[x10,%[v]]":: [v] "i" (mVRegistersOFF+16));
    asm volatile("stp d12,d13,[x10,%[v]]":: [v] "i" (mVRegistersOFF+32));
    asm volatile("stp d14,d15,[x10,%[v]]":: [v] "i" (mVRegistersOFF+48));

    asm volatile("mov x19,x10"); // copy this to x19 to preserve after execute

    //store return address after a switch
    asm volatile("str x11,[x10,%[v]]"::[v] "i" (mLROFF));
    asm volatile("ldrb w9, [x10,%[v]]"::[v] "i" (mSwitchedOFF));
    asm volatile("cbz w9,2f");
    asm volatile( "mov w9,#0" );
    asm volatile("strb w9,[x10,%[v]]"::[v] "i" (mSwitchedOFF) );
    //recover stack
    asm volatile ("ldr x11,[x10,%[v]]"::[v] "i" (mStackEndOFF):"x11" ); //is set by _resizeStack
    asm volatile ("ldr x9,[x10,%[v]]"::[v] "i" (mStackOFF):"x9" );
    asm volatile ("1: ldp x12,x13,[x11,#-16]!":::"x12","x13" ); 
    asm volatile ("stp x12,x13,[sp,#-16]!");
    asm volatile ("cmp x11,x9\n");
    asm volatile ("b.ne 1b");
    //unstack mandatory registers and lr

    asm volatile( "ldp d14,d15,[sp],#16");
    asm volatile( "ldp d12,d13,[sp],#16");
    asm volatile( "ldp d10,d11,[sp],#16");
    asm volatile( "ldp d8,d9,[sp],#16");

    asm volatile( "ldp x28,x29,[sp],#16");
    asm volatile( "ldp x26,x27,[sp],#16");
    asm volatile( "ldp x24,x25,[sp],#16");
    asm volatile( "ldp x22,x23,[sp],#16");
    asm volatile( "ldp x20,x21,[sp],#16"); 
    asm volatile( "ldp lr,x19,[sp],#16");

    //jump to context switch point
    asm volatile( "br lr"); //si hago ret valdría??

    
    asm volatile("2:");
  
    execute( msegs );
//    asm volatile( "continueexecuting:");
continueexecuting:
    asm volatile("mov x10,x19"); //temporary, because X10 could have changed
    //recover mandatory registers
    asm volatile("ldp x19,x20,[x10,%[v]]":: [v] "i" (mRegistersOFF));
    asm volatile("ldp x21,x22,[x10,%[v]]":: [v] "i" (mRegistersOFF+16));
    asm volatile("ldp x23,x24,[x10,%[v]]":: [v] "i" (mRegistersOFF+32));
    asm volatile("ldp x25,x26,[x10,%[v]]":: [v] "i" (mRegistersOFF+48));
    asm volatile("ldp x27,x28,[x10,%[v]]":: [v] "i" (mRegistersOFF+64));
  //  asm volatile("ldp x29,x30,[x10,%[v]]":: [v] "i" (mRegistersOFF+80));
    asm volatile("ldr x29,[x10,%[v]]":: [v] "i" (mRegistersOFF+80));
    //Restore V registers
    asm volatile("ldp d8,d9,[x10,%[v]]":: [v] "i" (mVRegistersOFF));
    asm volatile("ldp d10,d11,[x10,%[v]]":: [v] "i" (mVRegistersOFF+16));
    asm volatile("ldp d12,d13,[x10,%[v]]":: [v] "i" (mVRegistersOFF+32));
    asm volatile("ldp d14,d15,[x10,%[v]]":: [v] "i" (mVRegistersOFF+48));
}

volatile void fakeFunction( ) __attribute__((noinline))  __attribute__( (used)) __attribute__((naked));
volatile void fakeFunction( )
{
    //x0 = process.
    asm volatile("wrapperSwitch:");
    //stack lr and mandatory registers to preserve
    asm volatile("stp lr,x19,[sp,#-16]!" );
    asm volatile("stp x20,x21,[sp,#-16]!" );
    asm volatile("stp x22,x23,[sp,#-16]!" );
    asm volatile("stp x24,x25,[sp,#-16]!" );
    asm volatile("stp x26,x27,[sp,#-16]!" );
    asm volatile("stp x28,x29,[sp,#-16]!" );
    asm volatile("stp d8,d9,[sp,#-16]!" );
    asm volatile("stp d10,d11,[sp,#-16]!" );
    asm volatile("stp d12,d13,[sp,#-16]!" );
    asm volatile("stp d14,d15,[sp,#-16]!" );
    //set as "switched"
	asm volatile ( "mov w1,#1");
	asm volatile ("strb w1, [x0,%[v]]":: [v] "i" (mSwitchedOFF) );
    //calculate stack size
    asm volatile( "ldr x20,[x0,%[v]]":: [v] "i" (mIniSPOFF)); //SP at function begin
    asm volatile( "mov x1,x20"); //temp copy
    asm volatile( "mov x19,sp"); //temporary because sp can not be used in sub
    asm volatile( "sub x1,x1,x19");
    asm volatile( "mov x19,x0"); //save temporary
	asm volatile( "bl _resizeStack" );
	asm volatile( "mov x0,x19"); //restore x0 (=this)
    //save stack
    //inisp is in x20
	asm volatile ("ldr x2,[x0,%[v]]":: [v] "i" (mStackOFF)); //local stack base
    asm volatile( "mov x5,#0");//prueba
    asm volatile("4:ldp x3,x4,[sp],#16"); //increment after
    asm volatile("stp x3,x4,[x2],#16");
    asm volatile("add x5,x5,#1");
    asm volatile("cmp sp,x20");
    asm volatile("b.ne 4b");
    
    //now return to start point
    asm volatile( "ldr x2,[x0,%[v]]"::[v] "i" (mLROFF));
    asm volatile( "br x2 "); //no estoy seguro si RET x2 me valdría o sería el correcto
}

void Process::_switchProcess( )
{
	Process* p = ProcessScheduler::getCurrentProcess();
    MThreadAttributtes* mt = p;

    //x0 = p
    asm volatile( "ldr x0,%0"::"m" (mt):"x0");
    asm volatile( "bl wrapperSwitch" );
}

#endif

