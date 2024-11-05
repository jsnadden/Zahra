#pragma once

#include "Zahra/Renderer/Shader.h"

#include <filesystem>
#include <vulkan/vulkan.h>

namespace Zahra
{
	namespace VulkanShaderResources
	{
		struct UniformBuffer
		{
			std::string Name;
			uint32_t Binding;
			size_t ByteSize;
			size_t MemberCount;
		};
		
	}

	struct VulkanShaderReflectionData
	{
		std::vector<VulkanShaderResources::UniformBuffer> UniformBuffers;
	};

	class VulkanShader : public Shader
	{
	public:
		VulkanShader(ShaderSpecification& specification);
		~VulkanShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual const std::string& GetName() const override { return m_Specification.Name; }

		const std::vector<VkPipelineShaderStageCreateInfo>& GetPipelineShaderStageInfos() const { return m_PipelineShaderStageInfos; }

	private:
		ShaderSpecification m_Specification;

		std::unordered_map<ShaderStage, std::string> m_GLSLSource;
		std::unordered_map<ShaderStage, std::vector<uint32_t>> m_SPIRVBytecode, m_SPIRVBytecode_Debug;
		std::unordered_map<ShaderStage, VulkanShaderReflectionData> m_ReflectionData;

		std::vector<VkPipelineShaderStageCreateInfo> m_PipelineShaderStageInfos;

		bool ReadShaderSource(ShaderStage stage);
		void CompileOrGetSPIRV(std::unordered_map<ShaderStage, std::vector<uint32_t>>& bytecode, const std::filesystem::path& cacheDirectory, bool debug);
		void Reflect();
		void CreateModules();

		std::string GetSourceFilename(ShaderStage stage);
	};

}

