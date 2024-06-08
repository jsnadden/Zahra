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

#ifdef Z_DEBUG
	#define Z_ENABLE_ASSERTS
#endif

#ifdef Z_ENABLE_ASSERTS
	#define Z_ASSERT(x, ...) { if(!(x)) { Z_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define Z_CORE_ASSERT(x, ...) { if (!(x)) { Z_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define Z_ASSERT(x, ...)
	#define Z_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define Z_BIND_EVENT_FN(f) std::bind(&f, this, std::placeholders::_1)