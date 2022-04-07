#ifdef MEL_X86_GCC


No vale. usar la de x64 que ya esta no tiene mucho sentido ademas
#include <tasking/Process.h>
using mel::tasking::Process;
using mel::tasking::MThreadAttributtes;

#include <tasking/ProcessScheduler.h>


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
	tamanoActual = (( (char*)mStackEnd- (char*)mActualSP))>>2;
	if ( !mSleeped )
	{
        MThreadAttributtes* realThis = this;
		if ( mReturnInmediately )
		{
			//restauro periodo
			setPeriod( mRealPeriod );
			mReturnInmediately = false;
		}
        asm volatile( "mov eax,%0"::"m" (realThis) :"eax","memory");
		asm volatile( "mov [eax + %[v]],esp"::[v] "J" (mIniSPOFF):"eax" );//guardo sp en spIni
        asm volatile( "mov [eax + %[v]],ebp"::[v] "J" (mIniBPOFF):"eax"  );//guardo el ebp
        asm volatile( "cmp dword ptr [eax+%[v]], 0"::[v] "J" (mSwitchedOFF):"eax" ); //�false?
        //asm volatile( "jz $%[v]"::[v] "J" (+38) );  //sigue ejecutando
        //asm volatile( "jz $+38" );  //sigue ejecutando
        asm volatile( "jz continueExecuting");
        asm volatile( "mov dword ptr [eax+%[v]], 0"::[v] "J" (mSwitchedOFF) :"eax");//lo pongo a false

			//recupero pila
        asm volatile( "std " );//para decrementar
        asm volatile( "mov ecx,%0"::"m" (tamanoActual):"ecx","memory" );
        asm volatile( "sub esp,4":::"esp" );
        asm volatile( "mov edi,esp":::"esp" );
        asm volatile( "mov esi,dword ptr [eax + %[v]]"::[v] "J" (mStackEndOFF):"esp" );
        asm volatile( "sub esi,4":::"esp" );
        asm volatile( "rep movsd" );

        asm volatile( "mov esp,edi":::"esp" );
        asm volatile( "add esp,4":::"esp" );//ya que edi queda decrementado
        asm volatile( "cld" );  //parece ser que por defecto est� as�..�seguro?
        asm volatile( "pop edi":::"edi" );
        asm volatile( "pop esi":::"edi" );
        asm volatile( "pop ebx":::"ebx" );
        asm volatile( "pop ebp");
        asm volatile( "ret" );
        asm volatile( "continueExecuting:":::"memory");


        _execute( msegs );
      	//chapuza, hacerlo m�s �ptimo
		asm volatile( "mov eax,%0"::"m" (realThis):"eax","memory");
        asm volatile( "mov ecx,[eax + %[v]]"::[v] "J" (mIniSPOFF):"eax","eax");
			//sub ecx,4
        asm volatile( "sub ecx,esp":::"ecx","esp" );   //me da el tama�o nuevo
        asm volatile( "mov edx, [eax + %[v]]"::[v] "J" (mStackEndOFF):"eax" );
			//sub edx,4
        asm volatile( "sub edx,ecx":::"edx","ecx" );
        asm volatile( "sub edx,8":::"edx" );
        asm volatile( "mov dword ptr [eax + %[v]],edx"::[v] "J" (mActualSPOFF):"eax" );
        asm volatile( "mov esp, dword ptr [eax + %[v]]"::[v] "J" (mIniSPOFF):"esp","eax" );  //recupero el sp

	}
}
volatile void fakeFunction() __attribute__((optimize(0)));
volatile void fakeFunction()
{

    asm volatile("wrapperSwitch:");
    asm volatile( "push ebp\n" );
    asm volatile( "push ebx\n":::"ebx");
    asm volatile( "push esi\n":::"esi" );
    asm volatile( "push edi\n":::"edi" );
    asm volatile("mov eax, [esp+20]\n":::"eax");
    asm volatile(
                "mov dword ptr [eax+%[v]], 1\n"::[v] "J" (mSwitchedOFF):"eax" );
    asm volatile("mov ecx,[eax+%[v]]\n"::[v] "J" (mIniSPOFF):"ecx" );
    asm volatile(
                "mov esi,ecx\n"
                "sub esi,4\n"
                "sub ecx, esp\n":::"esi","ecx","esp"
        );

        asm volatile(
			//llamada a recrece pila
		"cld\n"
		"push ecx\n"
		"push eax\n"
		"call _mel_tasking_resizeStack\n":::"ecx","eax");
    asm volatile(
		//add esp,8
		//REcupero valores de ecx y eax
		"pop eax\n"
		"pop ecx\n":::"eax","ecx"
        );
    asm volatile(
		"std\n"  //decrementa
		"shr ecx,2\n" //tama�o en dwords
		:::"ecx"
		);
    asm volatile(
		"mov edi,dword ptr [eax + %[v]]\n"::[v] "J" (mStackEndOFF):"edi" );
    asm volatile(
        "sub edi,4\n"  //uff que cutre me est� quedando
		"rep movsd\n"
		"cld\n" //parece ser que por defecto est� as�..Mirar calling_conventions.pdf
		"add edi,4\n"  // ya que se queda decrementado el edi
		:::"edi"
    );
    asm volatile(
    		//tengo comprobado que est� bien copiada la pila y la cima correcta.
		"mov dword ptr [eax + %[v]],edi\n"::[v] "J" (mActualSPOFF)
        :"eax","edi"
		);
    asm volatile(
			//recupero sp inicial
		"mov esp, dword ptr [eax + %[v]]\n"::[v] "J" (mIniSPOFF):"esp","eax"
		);
    asm volatile(
    //recupero registros guardados
		"pop edi\n"
		"pop esi\n"
		"pop ebx\n"
		:::"edi","esi","ebx");
    asm volatile(
		"mov esp, dword ptr [eax + %[v]]\n"::[v] "J" (mIniBPOFF) //as� me cepillo las variables locales
            :"eax"
		);
    asm volatile(
		"pop ebp\n"
		"ret 4\n"  //desapila el parametro msegs-->�las func miembro son stdcall!
    );
}

volatile void mel::core::switchProcess( bool returnInmediately)
{
    Process* p = ProcessScheduler::getCurrentProcess();
    MThreadAttributtes* mt = p;
    p->mReturnInmediately = returnInmediately;
    if ( returnInmediately )
    {
        mt->mRealPeriod = p->getPeriod();
        p->setPeriod( 0 );
    }
    asm volatile( "push dword ptr %0\n"::"m" (mt):"memory");
    asm volatile( "call wrapperSwitch");
    asm volatile( "add esp,4");
}

#endif

