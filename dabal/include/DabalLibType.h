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

#endif
