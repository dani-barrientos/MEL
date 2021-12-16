#ifdef _MSC_VER
#include <core/Process.h>
using core::Process;
using core::MThreadAttributtes;

#include <core/ProcessScheduler.h>
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

volatile void Process::checkMicrothread( unsigned int msegs )
{
//NO ME CUADRA QUE ESTÉ BIEN. NO ESTOY GUARDANDO BX NI NADA DE ESO... TENGO QUE REVISAR LA CALLING CONVENTION
	//tendria que guardar esi,edi,ebx Me da que tal vez estoy asumiendo (y tal vez sea cierto) que siempre hace el pushdi,etc en el mismo orden
	register int tamanoActual;
	tamanoActual = (int)(( (char*)mStackEnd- (char*)mActualSP))>>2;
	
    register MThreadAttributtes* realThis = this;
	_asm
	{
//prueba

	/*	lea eax,returnAddress
		push eax  //truqui para no cambiar el switch anterior
		push eax
		push ebp;
		push ebx;
		push esi;
		push edi;
		*/

		mov eax,realThis;
		mov [eax + MThreadAttributtes::mIniSP],esp;//guardo sp en spIni
		mov [eax + MThreadAttributtes::mIniBP],ebp;//guardo el ebp
		cmp byte ptr [eax+MThreadAttributtes::mSwitched], 0;
    //asm volatile( "jz $%[v]"::[v] "J" (+38) );  //sigue ejecutando
    //asm volatile( "jz $+38" );  //sigue ejecutando
		jz continueExecuting;
		mov byte ptr [eax+MThreadAttributtes::mSwitched], 0;

		//recupero pila
		std;//para decrementar
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
	/*	pop edi;
		pop esi;
		pop ebx;
		pop ebp;
		pop eax
		pop eax
		*/
	}


    execute( msegs );
	_asm
	{
   	//chapuza, hacerlo más óptimo TODO esto realmente no deberia estar bien, puesto que eax no tiene por qué mantenerse
	mov eax,realThis;
	mov ecx,[eax + MThreadAttributtes::mIniSP];
		//sub ecx,4
    sub ecx,esp
	mov edx, [eax + MThreadAttributtes::mStackEnd];
		//sub edx,4
    sub edx,ecx;
    sub edx,8;
    mov dword ptr [eax + MThreadAttributtes::mActualSP],edx;
    mov esp, dword ptr [eax + MThreadAttributtes::mIniSP];
	}
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
	//llamada a recrece pila
	cld;
	push ecx;
	push eax;
	call resizeStack;
    //add esp,8
		//REcupero valores de ecx y eax
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
    //recupero sp inicial
	mov esp, dword ptr [eax + MThreadAttributtes::mIniSP];
	pop edi;
	pop esi;
	pop ebx;
	mov esp, dword ptr [eax + MThreadAttributtes::mIniBP];
	pop ebp;
	ret 4;
	}
}

void Process::_switchProcess( ) OPTIMIZE_FLAGS
{

	auto p = ProcessScheduler::getCurrentProcess();
    MThreadAttributtes* mt = p.get();
	switchHelper( mt );
}
#pragma optimize("",on)
#endif

