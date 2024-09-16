#pragma once

#include "Zahra/Renderer/Shader.h"

#include <filesystem>
#include <vulkan/vulkan.h>

namespace Zahra
{


	class VulkanShader
	{
	public:
		VulkanShader(const std::string& name, const std::filesystem::path& directory);

	private:
		uint32_t m_RendererID;

		std::string m_Name;
		std::filesystem::path m_SourceDirectory;

		std::unordered_map<VkShaderStageFlagBits, std::string> m_GLSLSource;
		std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>> m_SPIRVBytecode;

		void CompileOrGetBinaries(const std::unordered_map<VkShaderStageFlagBits, std::string>& shaderSources);
		void CreateProgram();
		void Reflect(VkShaderStageFlagBits stage, const std::vector<uint32_t>& shaderData);
	};

}

