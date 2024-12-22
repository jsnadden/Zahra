#pragma once

#include <filesystem>

namespace Zahra
{
	struct RendererConfig
	{
		// Swapchain
		uint32_t FramesInFlight = 3;

		// Shaders
		bool ForceShaderCompilation = false;

		// 2d batch renderer
		uint32_t MaxBatchSize = 10000;
		uint32_t MaxTextureSlots = 32;

		// TODO: these really belong in an AssetManagerConfig
		std::filesystem::path ShaderSourceDirectory		= "Resources/Shaders";
		std::filesystem::path ShaderCacheDirectory		= "Cache/Shaders";
		std::filesystem::path MeshSourceDirectory		= "Assets/Models";
		std::filesystem::path TextureSourceDirectory	= "Assets/Textures";
	};

}
