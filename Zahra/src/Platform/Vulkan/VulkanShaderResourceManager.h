#pragma once

#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "Zahra/Renderer/ShaderResourceManager.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	
	struct VulkanShaderResource
	{
		std::vector<Ref<RefCounted>> Data; // vector to account for array-type resources, size=1 otherwise
		VulkanShaderResourceMetadata Metadata;
	};	

	class VulkanShaderResourceManager : public ShaderResourceManager
	{
	public:
		VulkanShaderResourceManager(const ShaderResourceManagerSpecification& specification);
		~VulkanShaderResourceManager();

		virtual void ProvideResource(const std::string& name, Ref<UniformBufferSet> uniformBufferSet, uint32_t arrayIndex = 0) override;

		virtual bool CheckIfComplete() override;
		virtual void Bake() override;

		std::vector<VkDescriptorSet>& GetDescriptorSets();
		std::vector<VkDescriptorSet>& GetDescriptorSets(uint32_t frame) { return m_DescriptorSets[frame]; }

		uint32_t GetFirstSet() { return m_FirstSet; }
		uint32_t GetLastSet() { return m_LastSet; }

	private:
		Ref<VulkanShader> m_Shader;
		uint32_t m_FirstSet, m_LastSet;

		// resources organised by corresponding uniform name in shader code
		std::map<std::string, VulkanShaderResource> m_Resources;
		
		VkDescriptorPool m_DescriptorPool;

		// descriptors organised by (frame-in-flight, set)
		std::vector<std::vector<VkDescriptorSet>> m_DescriptorSets;

		void Init();
	};

}
