#pragma once

#include <memory>

// TODO: these macros should be defined externally. Doing it like this means you have to make sure this is included carefully!
// Platform detection using predefined macros
#ifdef _WIN32
	/* Windows x64/x86 */
	#ifdef _WIN64
		/* Windows x64  */
		#define Z_PLATFORM_WINDOWS
	#else
		/* Windows x86 */
		#error "x86 Builds are not supported!"
	#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>
	/* TARGET_OS_MAC exists on all the platforms
	 * so we must check all of them (in this order)
	 * to ensure that we're running on MAC
	 * and not some other Apple platform */
	#if TARGET_IPHONE_SIMULATOR == 1
		#error "IOS simulator is not supported!"
	#elif TARGET_OS_IPHONE == 1
		#define Z_PLATFORM_IOS
		#error "IOS is not supported!"
	#elif TARGET_OS_MAC == 1
		#define Z_PLATFORM_MACOS
		#error "MacOS is not supported!"
	#else
		#error "Unknown Apple platform!"
	#endif
 /* We also have to check __ANDROID__ before __linux__
  * since android is based on the linux kernel
  * it has __linux__ defined */
#elif defined(__ANDROID__)
	#define Z_PLATFORM_ANDROID
	#error "Android is not supported!"
#elif defined(__linux__)
	#define Z_PLATFORM_LINUX
	#error "Linux is not supported!"
#else
	/* Unknown compiler/platform */
	#error "Unknown platform!"
#endif

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