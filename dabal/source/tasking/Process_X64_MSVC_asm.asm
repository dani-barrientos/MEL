
    .code
    Attributes STRUCT
        IniRSP QWORD ?;
        StackEnd QWORD ?;
        Stack QWORD ?;        
        RegBP QWORD ?;
        RegBX QWORD ? ;
        RegSI QWORD ? ;
        RegDI QWORD ? ;
        Reg12 QWORD ?
        Reg13 QWORD ?
        Reg14 QWORD ?
        Reg15 QWORD ?
        ALIGN 16
        RegXMM6 OWORD ?
        RegXMM7 OWORD ?
        RegXMM8 OWORD ?
        RegXMM9 OWORD ?
        RegXMM10 OWORD ?
        RegXMM11 OWORD ?
        RegXMM12 OWORD ?
        RegXMM13 OWORD ?
        RegXMM14 OWORD ?
        RegXMM15 OWORD ?
        ;Switched BYTE ? ; cuidado con alineamientos,claro!!!
        Switched WORD ?  ;
        RegFPCSR WORD ?
        RegMXCSR DWORD ?;
        StackSize DWORD ?;
    Attributes ENDS
    EXTERN resizeStack:PROC
    ;EXTERN Process_execute:PROC

;void _checkMicrothread(MThreadAttributtes*,uint64_t msegs,void* executePtr);
_checkMicrothread PROC

    mov rax,rcx; //save rcx because it will be modified later
    mov [rcx+Attributes.RegBP],rbp; 
    mov [rcx+Attributes.RegBX],rbx;
    mov [rcx+Attributes.RegSI],rsi;
    mov [rcx+Attributes.RegDI],rdi;
    mov [rcx+Attributes.Reg12],r12;
    mov [rcx+Attributes.Reg13],r13;
    mov [rcx+Attributes.Reg14],r14;
    mov [rcx+Attributes.Reg15],r15;
    mov [rcx+Attributes.IniRSP],rsp;     
    ;SSE2 registers 128 bits no-volatile
    movdqa [rcx+Attributes.RegXMM6],xmm6
    movdqa [rcx+Attributes.RegXMM7],xmm7
    movdqa [rcx+Attributes.RegXMM8],xmm8
    movdqa [rcx+Attributes.RegXMM9],xmm9 
    movdqa [rcx+Attributes.RegXMM10],xmm10
    movdqa [rcx+Attributes.RegXMM11],xmm11
    movdqa [rcx+Attributes.RegXMM12],xmm12
    movdqa [rcx+Attributes.RegXMM13],xmm13
    movdqa [rcx+Attributes.RegXMM14],xmm14
    movdqa [rcx+Attributes.RegXMM15],xmm15
    fnstcw [rcx+Attributes.RegFPCSR]
    stmxcsr DWORD PTR [rcx+Attributes.RegMXCSR]; 
 
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
    
    movdqa xmm6,[rsp]
    movdqa xmm7,[rsp+16]
    movdqa xmm8,[rsp+32]
    movdqa xmm9,[rsp+48]
    movdqa xmm10,[rsp+64]
    movdqa xmm11,[rsp+80]
    movdqa xmm12,[rsp+96]
    movdqa xmm13,[rsp+112]
    movdqa xmm14,[rsp+128]
    movdqa xmm15,[rsp+144]
    ldmxcsr DWORD PTR [rsp+160]; 
    fldcw [rsp+164]
    add rsp,10*16+8
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp   
    ret

    continueExecuting_1:
    mov rcx,rax
    push r15
    xor r15,r15
    sub rsp,32 ;space for arguments, as defined by calling convention in MSVC
    ;check alignment
    test rsp,0fh
    jz callfunction
    mov r15,8
    sub rsp,8 ;     //align
callfunction:
    mov rcx,r8
    call r9
    ;call Process_execute
    add rsp,32
    add rsp,r15
    pop r15
    ret 
_checkMicrothread ENDP

;void _switchMT(MThreadAttributtes*)
_switchMT PROC
    push rbp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15

    ;at function call, stack must be aligned to 16 byte. rip "unalign it". so need to add another 8 bytes to the 10 xmm registers
    sub rsp,10*16+8
    movdqa [rsp],xmm6
    movdqa [rsp+16],xmm7
    movdqa [rsp+32],xmm8
    movdqa [rsp+48],xmm9
    movdqa [rsp+64],xmm10
    movdqa [rsp+80],xmm11
    movdqa [rsp+96],xmm12
    movdqa [rsp+112],xmm13
    movdqa [rsp+128],xmm14
    movdqa [rsp+144],xmm15
    stmxcsr DWORD PTR [rsp+160]; 
    fnstcw [rsp+164]
    
    mov r12,rcx ;MThreadAttributtes*
    mov [r12+Attributes.Switched],BYTE PTR 1
    mov r13,[r12+Attributes.IniRSP]
    mov r14,r13
    sub r14,8
    sub r13,rsp
    sub rsp,8
    mov rdx,r13  ;//size parameter in ecx 
    sub rsp,32 ;space for arguments, as defined by calling convention in MSVC
    mov r15,0
    ;check alignment
    test rsp,0fh
    jz callfunction
    mov r15,8
    sub rsp,8  ;if not aligned to 16, at least is aligned to 8 (sure??)
callfunction:
    call resizeStack
    mov rcx,r13
    mov rsi,r14
    add rsp,32
    add rsp,r15 ;add alignment
    
    std
    shr rcx,3
    mov rdi,[r12+Attributes.StackEnd]
    sub rdi,8
    rep movsq
    cld          
    mov rsp,[r12+Attributes.IniRSP] 
    mov rbp,[r12+Attributes.RegBP]         
    mov rbx,[r12+Attributes.RegBX]
    mov rsi,[r12+Attributes.RegSI]
    mov rdi,[r12+Attributes.RegDI]
    mov r13,[r12+Attributes.Reg13]
    mov r14,[r12+Attributes.Reg14]
    mov r15,[r12+Attributes.Reg15]
    movdqa xmm6,[r12+Attributes.RegXMM6]
    movdqa xmm7,[r12+Attributes.RegXMM7]
    movdqa xmm8,[r12+Attributes.RegXMM8]
    movdqa xmm9,[r12+Attributes.RegXMM9] 
    movdqa xmm10,[r12+Attributes.RegXMM10]
    movdqa xmm11,[r12+Attributes.RegXMM11]
    movdqa xmm12,[r12+Attributes.RegXMM12]
    movdqa xmm13,[r12+Attributes.RegXMM13]
    movdqa xmm14,[r12+Attributes.RegXMM14]
    movdqa xmm15,[r12+Attributes.RegXMM15]
    fldcw [r12+Attributes.RegFPCSR]; 
    ldmxcsr DWORD PTR [r12+Attributes.RegMXCSR]; 
    mov r12,[r12+Attributes.Reg12] 
    ret
_switchMT ENDP 
   END