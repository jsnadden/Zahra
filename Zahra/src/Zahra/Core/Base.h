#pragma once

#include <memory>

#include "PlatformDetection.h"

#ifdef Z_DEBUG
	#define Z_ENABLE_ASSERTS
#endif

// TODO: the assert macros should be capable of taking only a boolean input, without __VA_ARGS__.
#ifdef Z_ENABLE_ASSERTS
	#define Z_ASSERT(x, ...) { if(!(x)) { Z_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define Z_CORE_ASSERT(x, ...) { if (!(x)) { Z_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define Z_ASSERT(x, ...)
	#define Z_CORE_ASSERT(x, ...)
#endif

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