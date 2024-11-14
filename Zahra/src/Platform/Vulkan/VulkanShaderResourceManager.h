#pragma once

#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	
	struct VulkanShaderResource
	{
		std::vector<Ref<RefCounted>> Data;
		ShaderResourceType Type = ShaderResourceType::None;

		VulkanShaderResource() = default;

		VulkanShaderResource(Ref<VulkanUniformBuffer> uniformBuffer)
			: Data(uniformBuffer), Type(ShaderResourceType::UniformBuffer) {}

		VulkanShaderResource(Ref<VulkanUniformBufferSet> uniformBufferSet)
			: Data(uniformBufferSet), Type(ShaderResourceType::UniformBufferSet) {}

		void Set(Ref<VulkanUniformBuffer> uniformBuffer, uint32_t index = 0)
		{
			Type = ShaderResourceType::UniformBuffer;
			Data[index] = uniformBuffer;
		}

		void Set(Ref<VulkanUniformBufferSet> uniformBufferSet, uint32_t index = 0)
		{
			Type = ShaderResourceType::UniformBufferSet;
			Data[index] = uniformBufferSet;
		}

		template <typename T>
		Ref<T> Get() { return Data.As<T>(); }
	};
	

	struct VulkanShaderResourceManagerSpecification
	{
		Ref<VulkanShader> m_Shader;
		uint32_t m_FirstSet, m_LastSet; // range of set indices to be managed
	};

	class VulkanShaderResourceManager
	{
	public:
		VulkanShaderResourceManager(const VulkanShaderResourceManagerSpecification& specification);

		Ref<VulkanUniformBuffer> GetUniformBuffer(const std::string& name);
		Ref<VulkanUniformBufferSet> GetUniformBufferSet(const std::string& name);

		bool CheckIfComplete();
		void Bake();

	private:
		VulkanShaderResourceManagerSpecification m_Specification;

		std::map<std::string, VulkanShaderResources::UniformBufferLayout> m_UniformBufferLayouts;
		std::map<std::string, VulkanShaderResources::Texture2DLayout> m_Texture2DLayouts;

		std::map<uint32_t, std::map<uint32_t, VulkanShaderResource>> m_Resources; // indexed by (set, binding)

		VkDescriptorPool m_DescriptorPool;

		void Init();
	};

}
