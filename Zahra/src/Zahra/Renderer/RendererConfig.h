#pragma once

#include <filesystem>

namespace Zahra
{
	struct RendererConfig
	{
		// Swapchain
		uint32_t DesiredFramesInFlight = 3;

		// Shaders
		bool ForceShaderRecompilation = false;

		// 2d batch renderer
		uint32_t MaxBatchSize = 10000;

		// antialiasing
		bool MSAA = true;
	};
}
