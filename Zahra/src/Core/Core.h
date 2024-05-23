#pragma once

#ifdef Z_PLATFORM_WINDOWS
	#ifdef Z_BUILD_DLL
		#define ZAHRA_API __declspec(dllexport)
	#else
		#define ZAHRA_API __declspec(dllimport)
	#endif
#else
	#error "Currently only windows is supported."
#endif