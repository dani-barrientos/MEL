
#include <core/Process.h>
using mel::core::Process;
using mel::core::MThreadAttributtes;

#include <core/ProcessScheduler.h>

#define mSwitchedOFF offsetof( MThreadAttributtes,mSwitched)
#define mIniSPOFF offsetof( MThreadAttributtes,mIniSP)
#define mLROFF offsetof( MThreadAttributtes,mLR)
#define mStackEndOFF offsetof( MThreadAttributtes,mStackEnd)
#define mStackSizeOFF offsetof( MThreadAttributtes,mStackSize)
#define mStackOFF offsetof( MThreadAttributtes,mStack)



volatile void Process::checkMicrothread(uint64_t msegs )
{
	if ( !mSleeped )
	{
		volatile MThreadAttributtes* thisAux = this;
		asm volatile( "ldr r2,%0":"+m" (thisAux) );

		asm volatile( "str sp,[r2,%[v]]":: [v] "J" (mIniSPOFF) ); //mInisP = sp TODO ESTE YA NO ME VALE??
		//mter en LR la etiqueta exitFunction para retornar en el switch
		asm volatile( "ldr r3,=exitFunction" );
		asm volatile( "str r3,[r2,%[v]]"::[v] "J" (mLROFF));//a donde retornar tras el switch
		asm volatile ("ldr r3, [r2,%[v]]"::[v] "J" (mSwitchedOFF) );//R2<--mSwitched
		asm volatile( "cmp r3,#0\n"
			" beq continueExecute" );
		asm volatile( "mov r3,#0" );
		asm volatile("str r3,[r2,%[v]]"::[v] "J" (mSwitchedOFF) ); //mSwitched = false
		//restore stack
	//	asm volatile( "add sp,sp,#4" ); //POR QUE???
		asm volatile ("ldr r1,[r2,%[v]]"::[v] "J" (mStackEndOFF) );  //r1 <-- stackend
		asm volatile ("ldr r3,[r2,%[v]]"::[v] "J" (mStackOFF) ); //r3 <-- inicio pila
		asm volatile ("restoreLoop: ldmdb r1!,{r4}" );
		asm volatile ("stmfd sp!,{r4}" );
		asm volatile ("cmp r1,r3" );
		asm volatile ("bne restoreLoop" );
		//pila restaurada. vuelvo a punto de switch
		asm volatile("ldmfd sp!,{r4-r11}" );
		asm volatile("ldmfd sp!,{pc}" );
			
		asm volatile("continueExecute:"); //si mSwited== 0 --> sigue ejecutando

		execute( msegs );
		asm volatile( "exitFunction:");
	}
}


volatile void fakeFunction( )
{	
	asm volatile("wrapperSwitch:");
	//save standard registers
	asm volatile( "stmfd sp!,{lr}" );
	asm volatile( "stmfd sp!, {r4-r11}" ); 
	
	asm volatile ( "mov r1,#1");
	asm volatile ("str r1, [r0,%[v]]"::[v] "J" (mSwitchedOFF) );//R2<--mSwitched
	//calculamos tama�o de la pila
	asm volatile( "ldr r1,[r0,%[v]]"::[v] "J" (mIniSPOFF)); //iniSP
	asm volatile( "sub r1,r1,sp");
	//asm volatile ( "add r1,r1,#4" ); //tama�o de pila
	//call recrece pila
	asm volatile( "stmfd sp!,{r0}" );
	asm volatile( "bl mel_tasking_resizeStack" ); 
	asm volatile( "ldmfd sp!,{r0}"); //restauro R0 = MThread
	//ahora copia la pila
	asm volatile ("ldr r2,[r0,%[v]]"::[v] "J" (mStackOFF) ); //inicio pila
	asm volatile ("ldr r3,[r0,%[v]]"::[v] "J" (mIniSPOFF) ); //inisp
	asm volatile( "copyloop: ldmia sp!,{r1}" );
	asm volatile( "stmia r2!,{r1}" );
	asm volatile( "cmp sp,r3" ); //no inclure [inisp]
	asm volatile ( "bne copyloop" );
	//dejo SP como estaba al principio
	//asm volatile( "ldr sp,[r0,#12]" ); SE SUPONE QUE EL BUCLE ANTERIOR YA LO HACE
	//asm volatile( "add sp,#20" ); //a pelo, tengo que buscar el est�ndard
	//retornar al punto de llamada
	asm volatile( "ldr pc,[r0,%[v]]"::[v] "J" (mLROFF) );

	
}
volatile void switchProcess( bool returnInmediately)
{
    Process* p = mel::core::ProcessScheduler::getCurrentProcess();
    MThreadAttributtes* mt = p;
    p->mReturnInmediately = returnInmediately;
    if ( returnInmediately )
    {
        mt->mRealPeriod = p->getPeriod();
        p->setPeriod( 0 );
    }
    asm volatile( "ldr r0,%0\n":"+m" (mt));
    asm volatile( "bl wrapperSwitch");
}
