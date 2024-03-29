#ifdef _MSC_VER
#include <tasking/Process.h>
using mel::tasking::Process;
using mel::tasking::MThreadAttributtes;

#include <tasking/ProcessScheduler.h>
#pragma optimize("",off)

/*TODO:
    - hacer los sleep, wait, syncobject..
*/
#define mSwitchedOFF offsetof( MThreadAttributtes,mSwitched)
#define mIniSPOFF offsetof( MThreadAttributtes,mIniSP)
//#define mLROFF offsetof( MThreadAttributtes,mLR)
#define mStackEndOFF offsetof( MThreadAttributtes,mStackEnd)
#define mStackSizeOFF offsetof( MThreadAttributtes,mStackSize)
#define mStackOFF offsetof( MThreadAttributtes,mStack)
#define mActualSPOFF offsetof( MThreadAttributtes,mActualSP)
#define mIniBPOFF offsetof( MThreadAttributtes,mIniBP)

volatile void Process::checkMicrothread( uint64_t msegs )
{
	int tamanoActual;
	tamanoActual = (int)(( (char*)mStackEnd- (char*)mActualSP))>>2;
	
    MThreadAttributtes* realThis = this;
	_asm
	{
		mov eax,realThis;
		mov [eax + MThreadAttributtes::mIniSP],esp;//guardo sp en spIni
		mov [eax + MThreadAttributtes::mIniBP],ebp;//guardo el ebp
		cmp byte ptr [eax+MThreadAttributtes::mSwitched], 0;
		jz continueExecuting;
		mov byte ptr [eax+MThreadAttributtes::mSwitched], 0;

		std;
		mov ecx,tamanoActual;
		sub esp,4;
		mov edi,esp;
		mov esi,dword ptr [eax + MThreadAttributtes::mStackEnd];
		sub esi,4;
		rep movsd;

		mov esp,edi;
		add esp,4;
		cld;  
		pop edi;
		pop esi;
		pop ebx;
		pop ebp;

		ret;
		continueExecuting:;
	}
	

    _execute( msegs );
	/*
	_asm
	{
	mov eax,realThis;
	mov ecx,[eax + MThreadAttributtes::mIniSP];
    sub ecx,esp
	mov edx, [eax + MThreadAttributtes::mStackEnd];
    sub edx,ecx;
    sub edx,8;
    mov dword ptr [eax + MThreadAttributtes::mActualSP],edx;
    mov esp, dword ptr [eax + MThreadAttributtes::mIniSP];
	}
	*/
	return;
	
}

//helper function to do switch
void _declspec( naked ) switchHelper( MThreadAttributtes* mt )
{
	_asm
	{
    push ebp;
    push ebx;
    push esi;
    push edi;
    mov eax, [esp+20];
	mov byte ptr [eax+MThreadAttributtes::mSwitched], 1;
    mov ecx,[eax+MThreadAttributtes::mIniSP];
	mov esi,ecx;
    sub esi,4;
    sub ecx, esp;
	cld;
	push ecx;
	push eax;
	call mel_tasking_resizeStack;
	pop eax;
	pop ecx;		
    std;
	shr ecx,2;
	mov edi,dword ptr [eax + MThreadAttributtes::mStackEnd];
    sub edi,4;
	rep movsd;
	cld;
	add edi,4;
	mov dword ptr [eax + MThreadAttributtes::mActualSP],edi;
    
	mov esp, dword ptr [eax + MThreadAttributtes::mIniSP];
	pop edi;
	pop esi;
	pop ebx;
	mov esp, dword ptr [eax + MThreadAttributtes::mIniBP];
	pop ebp;
	ret 8;
	}
}

void Process::_switchProcess( ) OPTIMIZE_FLAGS
{
	auto p = ProcessScheduler::getCurrentProcess().get();
    MThreadAttributtes* mt = p;
	switchHelper( mt );	
}
#pragma optimize("",on)
#endif

