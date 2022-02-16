#include <core/atomics.h>


#if defined(DABAL_MACOSX) && (TARGET_CPU_X86_64 || TARGET_CPU_X86)
    #if TARGET_CPU_X86_64 
    asm(
        ".globl _atomicIncrement\n"
        "_atomicIncrement:\n"
        "movl $1,%eax\n"
        "lock xaddl %eax,(%rdi)\n"
        "incl %eax\n"
        "ret"
        );
    asm(
        
        ".globl _atomicDecrement\n"
        "_atomicDecrement:\n"
        "movl $-1,%eax\n"
        "lock xaddl %eax,(%rdi)\n"
        "decl %eax\n"
        "ret"
        );
     
    #elif TARGET_CPU_X86
    asm(
        ".globl _atomicIncrement\n"
        "_atomicIncrement:\n"
        "movl $1,%eax\n"
        "lock xaddl %eax,(%ecx)\n"
        "incl %eax\n"
        "ret"
        );
    asm(
        
        ".globl _atomicDecrement\n"
        "_atomicDecrement:\n"
        "movl $-1,%eax\n"
        "lock xaddl %eax,(%ecx)\n"
        "decl %eax\n"
        "ret"
        );
    #endif

#elif (defined (DABAL_IOS) || defined(DABAL_ANDROID)) && !TARGET_IPHONE_SIMULATOR && defined(__arm__)
volatile int32_t core::atomicIncrement(volatile int32_t* var)
{
    asm volatile( "1:" );
    asm volatile( "ldrex r2,[r0]");
    asm volatile( "add r2,r2,#1");
    asm volatile( "strex r1,r2,[r0]":::"memory");
    asm volatile( "cmp r1,#0":::"cc" );
    asm volatile( "bne 1b" ); //retry until store correct
    asm volatile( "mov r0,r2"); //result
    asm volatile( "bx lr" );
}

volatile int32_t core::atomicDecrement(volatile int32_t* var)
{
    asm volatile( "1:" );
    asm volatile( "ldrex r2,[r0]");
    asm volatile( "add r2,r2,#-1");
    asm volatile( "strex r1,r2,[r0]":::"memory");
    asm volatile( "cmp r1,#0":::"cc" );
    asm volatile( "bne 1b" ); //retry until store correct
    asm volatile( "mov r0,r2"); //result
    asm volatile( "bx lr" );
}

#elif !TARGET_IPHONE_SIMULATOR && (defined(__arm64__) || defined(__aarch64__))

volatile int32_t core::atomicIncrement(volatile int32_t* var)
{
    asm volatile( "1:" );
    asm volatile( "ldxr w2,[x0]");
    asm volatile( "add w2,w2,#1");
    asm volatile( "stxr w1,w2,[x0]":::"memory");
    asm volatile( "cbnz w1,1b":::"cc" );
    asm volatile( "mov w0,w2"); //result
    asm volatile( "ret" );
}

volatile int32_t core::atomicDecrement(volatile int32_t* var)
{
    asm volatile( "1:" );
    asm volatile( "ldxr w2,[x0]");
    asm volatile( "sub w2,w2,#1");
    asm volatile( "stxr w1,w2,[x0]":::"memory");
    asm volatile( "cbnz w1,1b":::"cc" );
    asm volatile( "mov w0,w2"); //result
    asm volatile( "ret" );
}
#endif
