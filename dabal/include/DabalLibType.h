	// El siguiente bloque ifdef muestra la forma est�ndar de crear macros que facilitan
// la exportaci�n de archivos DLL. Todos los archivos de este archivo DLL se compilan con el s�mbolo JCORE_EXPORTS
// definido en la l�nea de comandos. Este s�mbolo no se debe definir en ning�n proyecto
// que utilice este archivo DLL. De este modo, otros proyectos cuyos archivos de c�digo fuente incluyan el archivo
// interpretan que las funciones JCORE_API se importan de un archivo DLL, mientras que este archivo DLL interpreta los s�mbolos
// definidos en esta macro como si fueran exportados.
#pragma once

#ifdef WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

	#ifdef DABAL_EXPORTS
		#define DABAL_API __declspec(dllexport)
	#elif defined (DABAL_IMPORTS)
		#define DABAL_API __declspec(dllimport)
	#else
		#define DABAL_API
	
	#endif
#else
#define DABAL_API
#define DABAL_API_STL
#endif

#if defined (_MACOSX) || defined(_IOS) || defined(DABAL_POSIX)
#include <pthread.h>
#endif