#if defined(_IOS) || defined(_MACOSX)
#include <TargetConditionals.h>
#endif

#if (TARGET_IPHONE_SIMULATOR || TARGET_OS_MAC) && TARGET_CPU_X86
//version para MAC y simulador de IPhone y IPad
#include <core/Process.h>

using core::Process;
using core::MThreadAttributtes;

#include <core/ProcessScheduler.h>


#define mSwitchedOFF offsetof( MThreadAttributtes,mSwitched)
#define mIniSPOFF offsetof( MThreadAttributtes,mIniSP)
#define mStackEndOFF offsetof( MThreadAttributtes,mStackEnd)
#define mStackSizeOFF offsetof( MThreadAttributtes,mStackSize)
#define mStackOFF offsetof( MThreadAttributtes,mStack)
#define mActualSPOFF offsetof( MThreadAttributtes,mActualSP)
#define mIniBPOFF offsetof( MThreadAttributtes,mIniBP)


volatile void Process::checkMicrothread( uint64_t msegs )
{

int tamanoActual;
	tamanoActual = (( (char*)mStackEnd- (char*)mActualSP))>>2; 

    MThreadAttributtes* realThis = this;
	
    asm volatile( "movl %[v],%%eax"::[v] "m" (realThis):"%eax","%ebx" ); //force ebx save	
    //lo de poner una P es una patra–a del GCC para que no pongo el $

    asm volatile( "movl %%esp,(%P[v])(%%eax)"::[v] "i" (mIniSPOFF)); 
    asm volatile( "mov %%ebp,(%P[v])(%%eax)"::[v] "i" (mIniBPOFF));
    asm volatile( "cmpb $0,(%P[v])(%%eax)"::[v] "i" (mSwitchedOFF) );
    asm volatile("jz continueExecuting");
    

    asm volatile( "movb $0,(%P[v])(%%eax)"::[v] "i" (mSwitchedOFF));
    asm volatile( "std\n"
        "movl %[v],%%ecx"::[v] "m" (tamanoActual) );
    asm volatile( "sub $4,%esp" );
    asm volatile("movl %%esp,%%edi\n":::"%edi");
    asm volatile("movl (%P[v])(%%eax),%%esi"::[v] "i" (mStackEndOFF):"%esi" );
    asm volatile("sub $4,%esi" );
    asm volatile("rep movsd");
    
    asm volatile("mov %%edi,%%esp":::"%esp");
    asm volatile( "addl $4,%esp");
    asm volatile( "cld\n"
                 "pop %ebx\n"
                 "pop %esi\n"
                 "pop %edi\n"
                 "pop %ebp");
    asm volatile ("ret" );
    asm volatile("continueExecuting:" );

 
        execute( msegs );
     	//chapuza, hacerlo más óptimo
    asm volatile("movl %P[v],%%eax"::[v] "m" (realThis));
    asm volatile("movl (%P[v])(%%eax),%%ecx"::[v] "i" (mIniSPOFF):"%ecx");
    asm volatile("sub %esp,%ecx");
    asm volatile("mov (%P[v])(%%eax),%%edx"::[v] "i" (mStackEndOFF):"%edx");
    asm volatile("sub %ecx,%edx");
    asm volatile("sub $8,%edx");
    asm volatile("movl %%edx,(%P[v])(%%eax)"::[v] "i" (mActualSPOFF));
    asm volatile("mov (%P[v])(%%eax),%%esp"::[v] "i" (mIniSPOFF));


}
//!@note: optimize attribute ignored by clangºº
//volatile void fakeFunction() __attribute__((optimize(0)));
volatile void fakeFunction()
{
	asm volatile( "wrapperSwitch:");
    asm volatile( "push %ebp\n"
                 "push %edi\n"
                 "push %esi\n"
                 "push %ebx\n");
    asm volatile("movl 20(%esp),%eax");
    asm volatile("movb $1,(%P[v])(%%eax)"::[v] "i" (mSwitchedOFF));
    asm volatile("mov (%P[v])(%%eax),%%ecx"::[v] "i" (mIniSPOFF));
    asm volatile("movl %ecx,%esi");
    asm volatile("subl $4,%esi");
    asm volatile("sub %esp,%ecx");
    asm volatile( "cld\n"
                 "subl $4,%esp\n"
                 "push %ecx\n"
                 "push %eax\n"
                 "call _resizeStack");
    asm volatile("pop %eax\n"
                 "pop %ecx\n"
                 "add $4,%esp\n"
                 ); //16 byte alingment
    asm volatile("std\n"
                 "shr $2,%ecx");
    asm volatile("movl (%P[v])(%%eax),%%edi"::[v] "i" (mStackEndOFF) );
    asm volatile("sub $4,%edi");
    asm volatile("rep\n"
                 "movsd\n"
                 "cld");
    asm volatile("add $4,%edi");
    asm volatile("movl %%edi,(%P[v])(%%eax)"::[v] "i" (mActualSPOFF));
 
    asm volatile("movl (%P[v])(%%eax),%%esp"::[v] "i" (mIniBPOFF));
    asm volatile("sub $12,%esp\n"
                 "pop %esi\n"
                 "pop %edi\n"
                 "pop %ebx\n"
                 "pop %ebp\n"
                 "ret" );
    

}

void Process::_switchProcess( )
{
    Process* p = ProcessScheduler::getCurrentProcess();
    MThreadAttributtes* mt = p;
  
    asm volatile( "sub $12,%esp" );
	asm volatile( "pushl %[v]"::[v] "m" (mt) );
    asm volatile( "call wrapperSwitch" );
    asm volatile( "add $16,%esp");

}
#endif

