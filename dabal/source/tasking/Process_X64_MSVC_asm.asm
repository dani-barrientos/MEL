
    .code
    Attributes STRUCT
        IniRSP QWORD ?;
        StackEnd QWORD ?;
        Stack QWORD ?;
        RegBP QWORD ?;
        RegBX QWORD ? ;
        Reg12 QWORD ?
        Reg13 QWORD ?
        Reg14 QWORD ?
        Reg15 QWORD ?
        ;Switched BYTE ? ; cuidado con alineamientos,claro!!!
        Switched DWORD ?
        StackSize DWORD ?;
    Attributes ENDS
    EXTERN resizeStack:PROC
    EXTERN Process_execute:PROC
;para pruebas
_checkMicrothread PROC

    mov rax,rcx; //save rcx because it will be modified later
    mov [rcx+Attributes.RegBX],rbx;
    mov [rcx+Attributes.Reg12],r12;
    mov [rcx+Attributes.Reg13],r13;
    mov [rcx+Attributes.Reg14],r14;
    mov [rcx+Attributes.Reg15],r15;
    mov [rcx+Attributes.IniRSP],rsp;
 ;@todo   creo que esto es un problema porque al llamarme aquí cambia el bp, así que creo que tendré que llamar con un jmp o similar
    ;mov [rcx+Attributes.RegBP],rbp; 
    mov [rcx+Attributes.RegBP],rsp ; COMO PARECE QUE EL RBP NO LO USA, VOY A USAR EL SP
    cmp BYTE PTR [rcx+Attributes.Switched],0
    je continueExecuting_1
    mov[rcx+Attributes.Switched],BYTE PTR 0
    std 
    mov ecx,[rax+Attributes.StackSize]
    shr ecx,3
    sub rsp,8
    mov rdi,rsp
    mov rsi,[rax+Attributes.StackEnd]
    sub rsi,8
    rep movsq
    mov rsp,rdi
    add rsp,8
    cld    
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret
    continueExecuting_1:
    mov rcx,rax
    sub rsp,32 ;space for arguments, as defined by calling convention in MSVC
    call Process_execute
    add rsp,32
    ret 
_checkMicrothread ENDP
;first argument, by convention, is in rcx
_checkMT  PROC
 
  ;save registers 
;  tengo que encontrar forma de no tener que redefinir el struct    
    mov rax,rcx; //save rcx because it will be modified later
    mov [rcx+Attributes.RegBX],rbx;
    mov [rcx+Attributes.Reg12],r12;
    mov [rcx+Attributes.Reg13],r13;
    mov [rcx+Attributes.Reg14],r14;
    mov [rcx+Attributes.Reg15],r15;
    mov [rcx+Attributes.IniRSP],rsp;
 ;@todo   creo que esto es un problema porque al llamarme aquí cambia el bp, así que creo que tendré que llamar con un jmp o similar
    ;mov [rcx+Attributes.RegBP],rbp; 
    mov [rcx+Attributes.RegBP],rsp ; COMO PARECE QUE EL RBP NO LO USA, VOY A USAR EL SP
    cmp BYTE PTR [rcx+Attributes.Switched],0
    je continueExecuting
    mov[rcx+Attributes.Switched],BYTE PTR 0
    std 
    mov ecx,[rax+Attributes.StackSize]
    shr ecx,3
    sub rsp,8
    mov rdi,rsp
    mov rsi,[rax+Attributes.StackEnd]
    sub rsi,8
    rep movsq
    mov rsp,rdi
    add rsp,8
    cld
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ;ret

    ; mov %[v],%%rax"::[v] "m" (realThis):"rax");
    ;  asm volatile("mov %%rbx,(%P[v])(%%rax)"::[v] "i" (mIniRBXOFF) );
    ;  asm volatile("mov %%r12,(%P[v])(%%rax)"::[v] "i" (mRegistersOFF) );
    ;  asm volatile("mov %%r13,(%P[v])(%%rax)"::[v] "i" (mRegistersOFF+8) );
    ;  asm volatile("mov %%r14,(%P[v])(%%rax)"::[v] "i" (mRegistersOFF+16) );
    ;  asm volatile("mov %%r15,(%P[v])(%%rax)"::[v] "i" (mRegistersOFF+24) );
    ;  asm volatile("mov %%rsp,(%P[v])(%%rax)"::[v] "i" (mIniSPOFF));
    ;  asm volatile("mov %%rbp,(%P[v])(%%rax)"::[v] "i" (mIniBPOFF));
    ;  asm volatile("cmpb $0,(%P[v])(%%rax)"::[v] "i" (mSwitchedOFF):"cc" );
    ;  asm volatile("jz continueExecuting");
    ;  asm volatile( "movb $0,(%P[v])(%%rax)"::[v] "i" (mSwitchedOFF));
    ;  asm volatile( "std\n");
    ;  asm volatile( "mov (%P[v])(%%rax),%%ecx"::[v] "i" (mStackSizeOFF) );
    ;  asm volatile( "shr $3,%ecx");
    ;  asm volatile( "sub $8,%rsp" );
    ;  asm volatile("mov %rsp,%rdi\n");
    ;  asm volatile("mov (%P[v])(%%rax),%%rsi"::[v] "i" (mStackEndOFF) );

    ;  asm volatile("sub $8,%%rsi":::"cc" );
    ;  asm volatile("rep movsq":::"%rdi","%rsi");
    ;  asm volatile("mov %rdi,%rsp");
    ;  asm volatile("add $8,%rsp");
    ;  asm volatile( "cld\n"
    ;               "pop %r15\n"
    ;               "pop %r14\n"
    ;               "pop %r13\n"
    ;               "pop %r12\n"
    ;               "pop %rbx\n"
    ;               "pop %rbp");
    ;  asm volatile ("ret" );
    continueExecuting:
 
    ret 
_checkMT  ENDP
_switchMT PROC
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15
    mov r12,rcx ;MThreadAttributtes*
    mov [r12+Attributes.Switched],BYTE PTR 1
    mov r13,[r12+Attributes.IniRSP]
    mov r14,r13
    sub r14,8
    sub r13,rsp
    sub rsp,8
    mov rdx,r13  ;//size parameter in ecx 
    call resizeStack
    mov rcx,r13
    mov rsi,r14
    std
    shr rcx,3
    mov rdi,[r12+Attributes.StackEnd]
    sub rdi,8
    rep movsq
    cld
    mov rsp,[r12+Attributes.RegBP] ;@todo funciona de chiripa
    mov rbx,[r12+Attributes.RegBX]
    mov r13,[r12+Attributes.Reg13]
    mov r14,[r12+Attributes.Reg14]
    mov r15,[r12+Attributes.Reg15]
    mov r12,[r12+Attributes.Reg12] 
    ;pop rbp no vale si no se guardó el marco de pila...
    ret

;  asm volatile( "push %rbp\n"
;      "push %rbx\n"
;      "push %r12\n"
;      "push %r13\n"
;      "push %r14\n"
;      "push %r15\n"
                  
;     );
;     asm volatile( "mov %rdi,%r12");
;     asm volatile("movb $1,(%P[v])(%%r12)"::[v] "i" (mSwitchedOFF));
;     asm volatile("mov (%P[v])(%%r12),%%r13"::[v] "i" (mIniSPOFF));
;     asm volatile("mov %r13,%r14");
;     asm volatile("sub $8,%r14");   
;     asm volatile("sub %rsp,%r13");
;     asm volatile( "sub $8,%rsp" );
;     asm volatile( "mov %r13,%rsi" );
;     asm volatile( "call resizeStack":::"memory");
;     asm volatile("mov %r13,%rcx");
;     asm volatile("mov %r14,%rsi");
    
;     asm volatile("std\n"
;                 "shr $3,%rcx");
;     asm volatile("mov (%P[v])(%%r12),%%rdi"::[v] "i" (mStackEndOFF) );
;     asm volatile("sub $8,%%rdi":::"cc");
;     asm volatile("rep movsq":::"%rdi","%rsi" );
;     asm volatile("cld");

;     asm volatile("mov (%P[v])(%%r12),%%rsp"::[v] "i" (mIniBPOFF));
;     asm volatile("mov (%P[v])(%%r12),%%rbx"::[v] "i" (mIniRBXOFF) );
;     asm volatile("mov (%P[v])(%%r12),%%r13"::[v] "i" (mRegistersOFF+8) );
;     asm volatile("mov (%P[v])(%%r12),%%r14"::[v] "i" (mRegistersOFF+16) );
;     asm volatile("mov (%P[v])(%%r12),%%r15"::[v] "i" (mRegistersOFF+24) );
;     asm volatile("mov (%P[v])(%%r12),%%r12"::[v] "i" (mRegistersOFF) );

;     asm volatile("pop %rbp");
;     asm volatile("ret");
_switchMT ENDP
    END
 