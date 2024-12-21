#pragma once

#include <filesystem>

namespace Zahra
{
	struct RendererConfig
	{
		// Swapchain
		uint32_t FramesInFlight = 3;

		// Shaders
		std::filesystem::path ShaderPath = "Resources/Shaders";
		bool ForceShaderCompilation = false;

		// Textures
		uint32_t MaximumBoundTextures = 32;

		// 2d batch renderer
		//uint32_t MaxBatchSize = 10000;
	};

}
