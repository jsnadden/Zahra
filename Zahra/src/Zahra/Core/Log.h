#pragma once

#include "Zahra/Core/Base.h"

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Zahra
{
	class Log
	{
	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;

	};

}

// Core logging macros
#define Z_CORE_TRACE(...) ::Zahra::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define Z_CORE_INFO(...) ::Zahra::Log::GetCoreLogger()->info(__VA_ARGS__)
#define Z_CORE_WARN(...) ::Zahra::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define Z_CORE_ERROR(...) ::Zahra::Log::GetCoreLogger()->error(__VA_ARGS__)
#define Z_CORE_CRITICAL(...) ::Zahra::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client logging macros
#define Z_TRACE(...) ::Zahra::Log::GetClientLogger()->trace(__VA_ARGS__)
#define Z_INFO(...) ::Zahra::Log::GetClientLogger()->info(__VA_ARGS__)
#define Z_WARN(...) ::Zahra::Log::GetClientLogger()->warn(__VA_ARGS__)
#define Z_ERROR(...) ::Zahra::Log::GetClientLogger()->error(__VA_ARGS__)
#define Z_CRITICAL(...) ::Zahra::Log::GetClientLogger()->critical(__VA_ARGS__)
