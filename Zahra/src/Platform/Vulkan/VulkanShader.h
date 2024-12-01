#pragma once

#include "Zahra/Renderer/Shader.h"
#include "Zahra/Renderer/VertexBuffer.h"

#include <filesystem>
#include <vulkan/vulkan.h>

namespace Zahra
{
	struct VulkanShaderAttribute
	{
		std::string Name;
		ShaderDataType Type;
		ShaderStage Stage;
		uint32_t Location;
		uint64_t ArrayLength = 1; // 1 for non-array types

		// TODO: handle structs and nested arrays
	};

	struct VulkanShaderResourceMetadata
	{
		std::string Name;
		ShaderResourceType Type;
		ShaderStage Stage;
		uint32_t Set;
		uint32_t Binding;
		uint64_t ByteSize;
		uint64_t MemberCount;
		uint64_t ArrayLength = 1; // 1 for non-array types
	};

	struct VulkanShaderReflectionData
	{
		std::vector<VulkanShaderAttribute> Attributes;
		std::vector<VulkanShaderResourceMetadata> ResourceMetadata;

		uint32_t MaxSetIndex = 0;
	};

	class VulkanShader : public Shader
	{
	public:
		VulkanShader(ShaderSpecification& specification, bool forceCompile);
		~VulkanShader();

		virtual const std::string& GetName() const override { return m_Specification.Name; }
		virtual const VertexBufferLayout& GetVertexLayout() const override { return m_VertexLayout; }

		const std::vector<VkPipelineShaderStageCreateInfo>& GetPipelineShaderStageInfos() const { return m_PipelineShaderStageInfos; }
		const std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts() const { return m_DescriptorSetLayouts; }

	private:
		ShaderSpecification m_Specification;

		std::unordered_map<ShaderStage, std::string> m_GLSLSource;
		std::unordered_map<ShaderStage, std::vector<uint32_t>> m_SPIRVBytecode, m_SPIRVBytecode_Debug;
		VulkanShaderReflectionData m_ReflectionData;

		VertexBufferLayout m_VertexLayout;

		std::vector<VkPipelineShaderStageCreateInfo> m_PipelineShaderStageInfos;
		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;

		bool ReadShaderSource(ShaderStage stage);
		void CompileOrGetSPIRV(std::unordered_map<ShaderStage, std::vector<uint32_t>>& bytecode, const std::filesystem::path& cacheDirectory, bool debug, bool forceCompile);
		void Reflect();
		void CreateVertexLayout();
		void CreateDescriptorSetLayouts();
		void CreateModules();

		std::string GetSourceFilename(ShaderStage stage);

		friend class VulkanShaderResourceManager;
	};

}

