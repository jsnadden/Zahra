#pragma once

#include <filesystem>

namespace Zahra
{
	struct RendererConfig
	{
		// Swapchain
		uint32_t DesiredFramesInFlight = 3;

		// Shaders
		bool ForceShaderCompilation = false;

		// 2d batch renderer
		uint32_t MaxBatchSize = 10000;

		// TODO: these really belong in an AssetManagerConfig
		std::filesystem::path ShaderSourceDirectory = "Resources/Shaders";
		std::filesystem::path ShaderCacheDirectory = "Cache/Shaders";
		std::filesystem::path MeshSourceDirectory = "Assets/Models";
	};
}
