#pragma once

#include "Zahra/Renderer/Shader.h"

#include <filesystem>
#include <vulkan/vulkan.h>

namespace Zahra
{


	class VulkanShader : public Shader
	{
	public:
		VulkanShader(ShaderSpecification& specification);
		~VulkanShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual const std::string& GetName() const override { return m_Specification.Name; }

		const std::vector<VkPipelineShaderStageCreateInfo>& GetPipelineShaderStages() { return m_PipelineShaderStageInfos; }

	private:
		ShaderSpecification m_Specification;

		std::unordered_map<Shader::Stage, std::string> m_GLSLSource;
		std::unordered_map<Shader::Stage, std::vector<uint32_t>> m_SPIRVBytecode;
		std::unordered_map<Shader::Stage, VkShaderModule> m_Modules;
		std::vector<VkPipelineShaderStageCreateInfo> m_PipelineShaderStageInfos;

		bool ReadShaderSource(Shader::Stage stage);
		void CompileOrGetSPIRV(const std::unordered_map<Shader::Stage, std::string>& shaderSources, const std::filesystem::path& cacheDirectory);
		void CreateModules(const std::unordered_map<Shader::Stage, std::vector<uint32_t>>& bytecode);

		std::string GetSourceFilename(Shader::Stage stage);
	};

}

