	// El siguiente bloque ifdef muestra la forma estándar de crear macros que facilitan
// la exportación de archivos DLL. Todos los archivos de este archivo DLL se compilan con el símbolo JCORE_EXPORTS
// definido en la línea de comandos. Este símbolo no se debe definir en ningún proyecto
// que utilice este archivo DLL. De este modo, otros proyectos cuyos archivos de código fuente incluyan el archivo
// interpretan que las funciones JCORE_API se importan de un archivo DLL, mientras que este archivo DLL interpreta los símbolos
// definidos en esta macro como si fueran exportados.
#pragma once

#ifdef WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

	#ifdef FOUNDATION_EXPORTS
		#define FOUNDATION_API __declspec(dllexport)
	#elif defined (FOUNDATION_IMPORTS)
		#define FOUNDATION_API __declspec(dllimport)
	#else
		#define FOUNDATION_API
	
	#endif
#else
#define FOUNDATION_API
#define FOUNDATION_API_STL
#endif

#if defined (_MACOSX) || defined(_IOS) || defined(_ANDROID)
#include <pthread.h>
#endif