#include "zpch.h"
#include "Log.h"

#pragma warning(push, 0)
	#include "spdlog/sinks/stdout_color_sinks.h"
	#include <spdlog/sinks/basic_file_sink.h>
#pragma warning(pop)

namespace Zahra
{
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	std::shared_ptr<spdlog::logger> Log::s_ScriptLogger;
	std::shared_ptr<spdlog::logger> Log::s_VulkanLogger;


	void Log::Init()
	{
		std::filesystem::path logDirectory = "./Logs";
		if (!std::filesystem::exists(logDirectory)) std::filesystem::create_directories(logDirectory);

		std::filesystem::path logFile = logDirectory / "Zahra.log";
		std::filesystem::path vkLogFile = logDirectory / "Vulkan.log";

		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile.string(), true));
		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		s_CoreLogger = std::make_shared<spdlog::logger>("ZAHRA", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_CoreLogger);
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->flush_on(spdlog::level::trace);

		s_ClientLogger = std::make_shared<spdlog::logger>("APP", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_ClientLogger);
		s_ClientLogger->set_level(spdlog::level::trace);
		s_ClientLogger->flush_on(spdlog::level::trace);

		s_ScriptLogger = std::make_shared<spdlog::logger>("DJINN", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_ScriptLogger);
		s_ScriptLogger->set_level(spdlog::level::trace);
		s_ScriptLogger->flush_on(spdlog::level::trace);

		std::vector<spdlog::sink_ptr> vkLogSinks;
		vkLogSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		vkLogSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(vkLogFile.string(), true));
		vkLogSinks[0]->set_pattern("%^[%T] %n: %v%$");
		vkLogSinks[1]->set_pattern("[%T] [%l] %n: %v");

		s_VulkanLogger = std::make_shared<spdlog::logger>("VULKAN", begin(vkLogSinks), end(vkLogSinks));
		spdlog::register_logger(s_VulkanLogger);
		s_VulkanLogger->set_level(spdlog::level::trace);
		s_VulkanLogger->flush_on(spdlog::level::trace);

	}
}
