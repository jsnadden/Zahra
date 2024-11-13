#pragma once

#include "Zahra/Renderer/Shader.h"

#include <filesystem>
#include <vulkan/vulkan.h>

namespace Zahra
{
	namespace VulkanShaderResources
	{
		struct UniformBufferData
		{
			std::string Name;
			VkShaderStageFlagBits Stages;
			uint32_t Set;
			uint32_t Binding;
			uint64_t ByteSize;
			uint64_t MemberCount;
			uint64_t ArrayLength = 1;
		};
		
	}

	struct VulkanShaderReflectionData
	{
		std::vector<VulkanShaderResources::UniformBufferData> UniformBuffers;

		uint32_t MaxSetIndex = 0;
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
		const std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts() const { return m_DescriptorSetLayouts; }

	private:
		ShaderSpecification m_Specification;

		std::unordered_map<ShaderStage, std::string> m_GLSLSource;
		std::unordered_map<ShaderStage, std::vector<uint32_t>> m_SPIRVBytecode, m_SPIRVBytecode_Debug;
		VulkanShaderReflectionData m_ReflectionData;

		std::vector<VkPipelineShaderStageCreateInfo> m_PipelineShaderStageInfos;
		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;

		bool ReadShaderSource(ShaderStage stage);
		void CompileOrGetSPIRV(std::unordered_map<ShaderStage, std::vector<uint32_t>>& bytecode, const std::filesystem::path& cacheDirectory, bool debug);
		void Reflect();
		void CreateDescriptorSetLayouts();
		void CreateModules();

		std::string GetSourceFilename(ShaderStage stage);
	};

}

