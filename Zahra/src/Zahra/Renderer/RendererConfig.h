#pragma once

#include <filesystem>

namespace Zahra
{
	struct RendererConfig
	{
		uint32_t FramesInFlight = 3;

		std::filesystem::path ShaderPath;
	};

}
