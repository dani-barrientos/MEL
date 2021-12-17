#if defined(_IOS) || defined(_MACOSX)
#include <TargetConditionals.h>
#endif

#if (TARGET_IPHONE_SIMULATOR || TARGET_OS_MAC) && TARGET_CPU_X86_64
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
#define mIniBPOFF offsetof( MThreadAttributtes,mIniRBP)
#define mIniRBXOFF offsetof( MThreadAttributtes,mIniRBX)
#define mRegistersOFF offsetof( MThreadAttributtes,mRegisters)


volatile void Process::checkMicrothread( uint64_t msegs )
{
     MThreadAttributtes* realThis = this;
     asm volatile( "mov %[v],%%rax"::[v] "m" (realThis):"rax");
     asm volatile("mov %%rbx,(%P[v])(%%rax)"::[v] "i" (mIniRBXOFF) );
     asm volatile("mov %%r12,(%P[v])(%%rax)"::[v] "i" (mRegistersOFF) );
     asm volatile("mov %%r13,(%P[v])(%%rax)"::[v] "i" (mRegistersOFF+8) );
     asm volatile("mov %%r14,(%P[v])(%%rax)"::[v] "i" (mRegistersOFF+16) );
     asm volatile("mov %%r15,(%P[v])(%%rax)"::[v] "i" (mRegistersOFF+24) );
     asm volatile("mov %%rsp,(%P[v])(%%rax)"::[v] "i" (mIniSPOFF));
     asm volatile("mov %%rbp,(%P[v])(%%rax)"::[v] "i" (mIniBPOFF));
     asm volatile("cmpb $0,(%P[v])(%%rax)"::[v] "i" (mSwitchedOFF) );
     asm volatile("jz continueExecuting");
     asm volatile( "movb $0,(%P[v])(%%rax)"::[v] "i" (mSwitchedOFF));
     asm volatile( "std\n");
     asm volatile( "mov (%P[v])(%%rax),%%ecx"::[v] "i" (mStackSizeOFF) );
     asm volatile( "shr $3,%ecx");
     asm volatile( "sub $8,%rsp" );
     asm volatile("mov %rsp,%rdi\n");
     asm volatile("mov (%P[v])(%%rax),%%rsi"::[v] "i" (mStackEndOFF) );

     asm volatile("sub $8,%rsi" );
     asm volatile("rep movsq");
     asm volatile("mov %rdi,%rsp");
     asm volatile("add $8,%rsp");
     asm volatile( "cld\n"
                  "pop %r15\n"
                  "pop %r14\n"
                  "pop %r13\n"
                  "pop %r12\n"
                  "pop %rbx\n"
                  "pop %rbp");
     asm volatile ("ret" );
     asm volatile("continueExecuting:" );
     execute( msegs );
}

volatile void fakeFunction()
{
    asm volatile( "wrapperSwitch:");
    asm volatile( "push %rbp\n"
     "push %rbx\n"
     "push %r12\n"
     "push %r13\n"
     "push %r14\n"
     "push %r15\n"
                  
    );
    asm volatile( "mov %rdi,%r12");
    asm volatile("movb $1,(%P[v])(%%r12)"::[v] "i" (mSwitchedOFF));
    asm volatile("mov (%P[v])(%%r12),%%r13"::[v] "i" (mIniSPOFF));
    asm volatile("mov %r13,%r14");
    asm volatile("sub $8,%r14");
   
    asm volatile("sub %rsp,%r13");
    asm volatile( "sub $8,%rsp" );
    asm volatile( "mov %r13,%rsi" );
    asm volatile( "call _resizeStack");
    asm volatile("mov %r13,%rcx");
    asm volatile("mov %r14,%rsi");
    
    asm volatile("std\n"
                "shr $3,%rcx");
    asm volatile("mov (%P[v])(%%r12),%%rdi"::[v] "i" (mStackEndOFF) );
    asm volatile("sub $8,%rdi");
    asm volatile("rep movsq");
    asm volatile("cld");

    asm volatile("mov (%P[v])(%%r12),%%rsp"::[v] "i" (mIniBPOFF));
    asm volatile("mov (%P[v])(%%r12),%%rbx"::[v] "i" (mIniRBXOFF) );
    asm volatile("mov (%P[v])(%%r12),%%r13"::[v] "i" (mRegistersOFF+8) );
    asm volatile("mov (%P[v])(%%r12),%%r14"::[v] "i" (mRegistersOFF+16) );
    asm volatile("mov (%P[v])(%%r12),%%r15"::[v] "i" (mRegistersOFF+24) );
    asm volatile("mov (%P[v])(%%r12),%%r12"::[v] "i" (mRegistersOFF) );

    asm volatile("pop %rbp");
    asm volatile("ret");
}
 
void Process::_switchProcess( )
{
    Process* p = ProcessScheduler::getCurrentProcess();
    MThreadAttributtes* mt = p;
    char needAlignment=false;
    asm volatile("test $0xf,%rsp");
    asm volatile("jz callfunction");
    asm volatile("movb $1,%[v]"::[v] "m" (needAlignment));
    asm volatile( "sub $8,%rsp"); 
    asm volatile( "callfunction:mov %[v],%%rdi"::[v] "m" (mt):"%rsi","%rdi" );
    asm volatile( "call wrapperSwitch" );
    asm volatile("cmpb $1,%[v]"::[v] "m" (needAlignment));
    asm volatile("jne endswitch");
    asm volatile( "add $8,%rsp");
    asm volatile("endswitch:");
 
}

#endif

