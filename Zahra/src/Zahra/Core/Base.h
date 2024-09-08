#pragma once

#include "PlatformDetection.h"

#include <memory>


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

namespace Zahra
{
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}


	// TODO: implement our own reference counting system, replacing shared_ptr
	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}


#include "Zahra/Core/Log.h"
#include "Zahra/Core/Assert.h"
