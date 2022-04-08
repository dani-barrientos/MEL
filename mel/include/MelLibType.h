#pragma once

#ifdef WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

	#ifdef MEL_EXPORTS
		#define MEL_API __declspec(dllexport)
	#elif defined (MEL_IMPORTS)
		#define MEL_API __declspec(dllimport)
	#else
		#define MEL_API
	
	#endif
#else
#define MEL_API

#endif
