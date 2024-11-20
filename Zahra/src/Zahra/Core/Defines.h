#pragma once

#include "PlatformDetection.h"

#define Z_VERSION_NUMBER(major, minor, patch) ((uint32_t)major << 20) | ((uint32_t)minor << 10) | ((uint32_t)patch)
#define Z_VERSION_TO_STRING(version) std::to_string(version >> 20) + "." + std::to_string((version >> 10) % (1 << 10)) + "." + std::to_string(version % (1 << 10))


#ifndef GLM_ENABLE_EXPERIMENTAL
	#define GLM_ENABLE_EXPERIMENTAL
#endif
#ifndef GLM_FORCE_RADIANS
	#define GLM_FORCE_RADIANS
#endif
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
	#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#ifdef Z_DEBUG
	#ifdef Z_PLATFORM_WINDOWS
		#define Z_DEBUGBREAK() __debugbreak()
	#elif defined(Z_PLATFORM_LINUX)
		#include <signal.h>
		#define Z_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif

	#define Z_ENABLE_ASSERTS
#else
	#define Z_DEBUGBREAK()
#endif


#define Z_EXPAND_MACRO(x) x
#define Z_STRINGIFY_MACRO(x) #x


#define BIT(x) (1 << x)


#define Z_BIND_EVENT_FN(f) [this](auto&&... args)->decltype(auto) { return this->f(std::forward<decltype(args)>(args)...); }


