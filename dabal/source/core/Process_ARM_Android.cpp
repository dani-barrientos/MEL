#ifdef DABAL_ANDROID

#include <core/Process.h>
using core::Process;
using core::MThreadAttributtes;

#include <core/ProcessScheduler.h>

#ifdef __arm__
	#define mSwitchedOFF offsetof( MThreadAttributtes,mSwitched)
	#define mIniSPOFF offsetof( MThreadAttributtes,mIniSP)
	#define mRegistersOFF offsetof( MThreadAttributtes,mRegisters)
	#define mCoRegistersOFF offsetof( MThreadAttributtes,mCoRegisters)
	#define mLROFF offsetof( MThreadAttributtes,mLR)
	#define mStackEndOFF offsetof( MThreadAttributtes,mStackEnd)
	#define mStackSizeOFF offsetof( MThreadAttributtes,mStackSize)
	#define mStackOFF offsetof( MThreadAttributtes,mStack)

volatile void Process::checkMicrothread(uint64_t msegs )
{
	volatile MThreadAttributtes* thisAux = this;
	asm volatile( "ldr r4,%[v]"::[v] "m" (thisAux):"r4","r0","r1" ); //tengo que enga�arle aqui para que tenga en cuenta que se modifican, ya que se hace cuando se produce un cambio de contexto

	asm volatile( "str sp, [r4,%[v]]":: [v] "i" (mIniSPOFF) ); 

	asm volatile( "add r4,r4,%[v]":: [v] "i" (mRegistersOFF):"r4" );
	//asm volatile( "stmia r4!,{r0-r12}");  //->en version iOS, est� mal ajustado el r4, debe apuntar a uno m�s
	//bloque funcional android
	asm volatile("stmia r4,{r0-r12}");
	//ajuste de posicion
	asm volatile( "add r4,r4,%[v]":: [v] "i" (mCoRegistersOFF-mRegistersOFF):"r4" );
	//fin bloque android

	asm volatile("vstmia r4,{d0-d15}");
	asm volatile( "sub r4,r4,%[v]":: [v] "i" (mCoRegistersOFF):"r4" );//restauramos posicion r4 al principio
	asm volatile( "str r4,[r4,%[v]]":: [v] "i" (mRegistersOFF+16) );//guar R4 en su posicion regitros + 4*4 --> MEJORAR

	asm volatile( "mov r5,#0\n" "add r5,pc,#64":::"r5" ); //TODO no se como obtener la direccion de un label!!
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

	asm volatile( "3:");
	asm volatile( "add r4,r4,%[v]":: [v] "i" (mRegistersOFF):"r4" );
	asm volatile( "ldmia r4,{r0-r3}":::"r0","r1","r2","r3");
	asm volatile( "add r4,r4,#20"); //apunto a R5
	//asm volatile( "ldmia r4!,{r5-r12}"); version iOS, no entiendo como funciona puesto que r4 tiene que quedar mal colocado (4 bytes menos), es decir, apunta a ultimo elemento	
	asm volatile("ldmia r4,{r5-r12}");
	//ajustar posicion
	asm volatile("add r4,r4,%[v]"::[v] "i" (mCoRegistersOFF - (mRegistersOFF + 20)) : "r4");
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
	asm volatile( "bl resizeStack" );
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
#elif defined(__aarch64__)
#define mSwitchedOFF offsetof( MThreadAttributtes,mSwitched)
#define mIniSPOFF offsetof( MThreadAttributtes,mIniSP)
#define mRegistersOFF offsetof( MThreadAttributtes,mRegisters)
#define mVRegistersOFF offsetof( MThreadAttributtes,mVRegisters)
#define mLROFF offsetof( MThreadAttributtes,mLR)
#define mStackEndOFF offsetof( MThreadAttributtes,mStackEnd)
#define mStackOFF offsetof( MThreadAttributtes,mStack)
volatile void Process::checkMicrothread(uint64_t msegs)
{
	
	volatile MThreadAttributtes* thisAux = this;
	void* ptr = &&continueexecuting; //direccion absoluta de etiqueta.
	asm volatile("ldr x10,%[v]"::[v] "m" (thisAux) : "x10", "x0", "x1"); //tengo que enga�arle aqui para que tenga en cuenta que se modifican, ya que se hace cuando se produce un cambio de contexto (y desde su punto de vista no hay nada de eso, por lo que el compilador podr�a no salvagaurdar estos y por tanto error al usarlos al retornar)
																		 //store return label address in local x11. Can be calcualted directly as a constant because address calculation if mor complicated, involving adr instruction
	asm volatile("ldr x11,%[v]"::[v] "m" (ptr) : "x11");
	asm volatile("mov x9,sp"); //sp to auxiliar register
	asm volatile("str x9, [x10,%[v]]"::[v] "i" (mIniSPOFF)); //save sp

	asm volatile("stp x19,x20,[x10,%[v]]"::[v] "i" (mRegistersOFF));
	asm volatile("stp x21,x22,[x10,%[v]]"::[v] "i" (mRegistersOFF + 16));
	asm volatile("stp x23,x24,[x10,%[v]]"::[v] "i" (mRegistersOFF + 32));
	asm volatile("stp x25,x26,[x10,%[v]]"::[v] "i" (mRegistersOFF + 48));
	asm volatile("stp x27,x28,[x10,%[v]]"::[v] "i" (mRegistersOFF + 64));
	// asm volatile("stp x29,x30,[x10,%[v]]":: [v] "i" (mRegistersOFF+80));
	asm volatile("str x29,[x10,%[v]]"::[v] "i" (mRegistersOFF + 80));

	//save SIMD registers. D = bottom 64 bit of v registers
	asm volatile("stp d8,d9,[x10,%[v]]"::[v] "i" (mVRegistersOFF));
	asm volatile("stp d10,d11,[x10,%[v]]"::[v] "i" (mVRegistersOFF + 16));
	asm volatile("stp d12,d13,[x10,%[v]]"::[v] "i" (mVRegistersOFF + 32));
	asm volatile("stp d14,d15,[x10,%[v]]"::[v] "i" (mVRegistersOFF + 48));

	asm volatile("mov x19,x10"); // copy this to x19 to preserve after execute

								 //store return address after a switch
	asm volatile("str x11,[x10,%[v]]"::[v] "i" (mLROFF));
	asm volatile("ldrb w9, [x10,%[v]]"::[v] "i" (mSwitchedOFF));
	asm volatile("cbz w9,2f");
	asm volatile("mov w9,#0");
	asm volatile("strb w9,[x10,%[v]]"::[v] "i" (mSwitchedOFF));
	//recover stack
	asm volatile ("ldr x11,[x10,%[v]]"::[v] "i" (mStackEndOFF) : "x11"); //is set by _resizeStack
	asm volatile ("ldr x9,[x10,%[v]]"::[v] "i" (mStackOFF) : "x9");
	asm volatile ("1: ldp x12,x13,[x11,#-16]!":::"x12", "x13");
	asm volatile ("stp x12,x13,[sp,#-16]!");
	asm volatile ("cmp x11,x9\n");
	asm volatile ("b.ne 1b");
	//unstack mandatory registers and lr

	asm volatile("ldp d14,d15,[sp],#16");
	asm volatile("ldp d12,d13,[sp],#16");
	asm volatile("ldp d10,d11,[sp],#16");
	asm volatile("ldp d8,d9,[sp],#16");

	asm volatile("ldp x28,x29,[sp],#16");
	asm volatile("ldp x26,x27,[sp],#16");
	asm volatile("ldp x24,x25,[sp],#16");
	asm volatile("ldp x22,x23,[sp],#16");
	asm volatile("ldp x20,x21,[sp],#16");
	asm volatile("ldp lr,x19,[sp],#16");

	//jump to context switch point
	asm volatile("br lr"); //si hago ret valdr�a??


	asm volatile("2:");

	execute(msegs);
	//    asm volatile( "continueexecuting:");
continueexecuting:
	asm volatile("mov x10,x19"); //temporary, because X10 could have changed
								 //recover mandatory registers
	asm volatile("ldp x19,x20,[x10,%[v]]"::[v] "i" (mRegistersOFF));
	asm volatile("ldp x21,x22,[x10,%[v]]"::[v] "i" (mRegistersOFF + 16));
	asm volatile("ldp x23,x24,[x10,%[v]]"::[v] "i" (mRegistersOFF + 32));
	asm volatile("ldp x25,x26,[x10,%[v]]"::[v] "i" (mRegistersOFF + 48));
	asm volatile("ldp x27,x28,[x10,%[v]]"::[v] "i" (mRegistersOFF + 64));
	//  asm volatile("ldp x29,x30,[x10,%[v]]":: [v] "i" (mRegistersOFF+80));
	asm volatile("ldr x29,[x10,%[v]]"::[v] "i" (mRegistersOFF + 80));
	//Restore V registers
	asm volatile("ldp d8,d9,[x10,%[v]]"::[v] "i" (mVRegistersOFF));
	asm volatile("ldp d10,d11,[x10,%[v]]"::[v] "i" (mVRegistersOFF + 16));
	asm volatile("ldp d12,d13,[x10,%[v]]"::[v] "i" (mVRegistersOFF + 32));
	asm volatile("ldp d14,d15,[x10,%[v]]"::[v] "i" (mVRegistersOFF + 48));

}

volatile void fakeFunction() __attribute__((noinline))  __attribute__((used)) __attribute__((naked));
volatile void fakeFunction()
{
	
	//x0 = process.
	asm volatile("wrapperSwitch:");
	
	//stack lr and mandatory registers to preserve
	asm volatile("stp lr,x19,[sp,#-16]!");
	asm volatile("stp x20,x21,[sp,#-16]!");
	asm volatile("stp x22,x23,[sp,#-16]!");
	asm volatile("stp x24,x25,[sp,#-16]!");
	asm volatile("stp x26,x27,[sp,#-16]!");
	asm volatile("stp x28,x29,[sp,#-16]!");
	asm volatile("stp d8,d9,[sp,#-16]!");
	asm volatile("stp d10,d11,[sp,#-16]!");
	asm volatile("stp d12,d13,[sp,#-16]!");
	asm volatile("stp d14,d15,[sp,#-16]!");
	//set as "switched"
	asm volatile ("mov w1,#1");
	asm volatile ("strb w1, [x0,%[v]]"::[v] "i" (mSwitchedOFF));
	//calculate stack size
	asm volatile("ldr x20,[x0,%[v]]"::[v] "i" (mIniSPOFF)); //SP at function begin
	asm volatile("mov x1,x20"); //temp copy
	asm volatile("mov x19,sp"); //temporary because sp can not be used in sub
	asm volatile("sub x1,x1,x19");
	asm volatile("mov x19,x0"); //save temporary
	asm volatile("bl resizeStack");
	asm volatile("mov x0,x19"); //restore x0 (=this)
								//save stack
								//inisp is in x20
	asm volatile ("ldr x2,[x0,%[v]]"::[v] "i" (mStackOFF)); //local stack base
	asm volatile("mov x5,#0");//prueba
	asm volatile("4:ldp x3,x4,[sp],#16"); //increment after
	asm volatile("stp x3,x4,[x2],#16");
	asm volatile("add x5,x5,#1");
	asm volatile("cmp sp,x20");
	asm volatile("b.ne 4b");

	//now return to start point
	asm volatile("ldr x2,[x0,%[v]]"::[v] "i" (mLROFF));
	asm volatile("br x2 "); //no estoy seguro si RET x2 me valdr�a o ser�a el correcto
	
}
void Process::_switchProcess()
{
	Process* p = ProcessScheduler::getCurrentProcess();
	MThreadAttributtes* mt = p;


	asm volatile("ldr x0,%0"::"m" (mt) : "x0");
	asm volatile("bl wrapperSwitch");
	
}
#endif //__arm__

#endif