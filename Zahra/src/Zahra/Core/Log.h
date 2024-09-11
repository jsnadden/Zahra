#pragma once

#include "Zahra/Core/Defines.h"

#pragma warning(push, 0)
	#include <spdlog/spdlog.h>
	#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

#include <glm/gtx/string_cast.hpp>

namespace Zahra
{
	class Log
	{
	public:
		static void Init();
		static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
		static std::shared_ptr<spdlog::logger>& GetScriptLogger() { return s_ScriptLogger; }
		static std::shared_ptr<spdlog::logger>& GetVulkanLogger() { return s_VulkanLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
		static std::shared_ptr<spdlog::logger> s_ScriptLogger;
		static std::shared_ptr<spdlog::logger> s_VulkanLogger;

	};

}

template <typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& stream, const glm::vec<L, T, Q>& vector)
{
	return stream << glm::to_string(vector);
}

template <typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& stream, const glm::mat<C, R, T, Q>& matrix)
{
	return stream << glm::to_string(matrix);
}

template <typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& stream, const glm::qua<T, Q>& quaternion)
{
	return stream << glm::to_string(quaternion);
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

// Script logging macros
#define Z_SCRIPT_TRACE(...) ::Zahra::Log::GetScriptLogger()->trace(__VA_ARGS__)
#define Z_SCRIPT_INFO(...) ::Zahra::Log::GetScriptLogger()->info(__VA_ARGS__)
#define Z_SCRIPT_WARN(...) ::Zahra::Log::GetScriptLogger()->warn(__VA_ARGS__)
#define Z_SCRIPT_ERROR(...) ::Zahra::Log::GetScriptLogger()->error(__VA_ARGS__)
#define Z_SCRIPT_CRITICAL(...) ::Zahra::Log::GetScriptLogger()->critical(__VA_ARGS__)

// Vulkan logging macros
#define Z_VULKAN_INFO(...)		::Zahra::Log::GetVulkanLogger()->info(__VA_ARGS__)
#define Z_VULKAN_WARN(...)		::Zahra::Log::GetVulkanLogger()->warn(__VA_ARGS__)
#define Z_VULKAN_ERROR(...)		::Zahra::Log::GetVulkanLogger()->error(__VA_ARGS__)
