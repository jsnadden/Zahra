#pragma once

#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Zahra/Renderer/ShaderResourceManager.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	
	struct VulkanShaderResource
	{
		std::vector<Ref<RefCounted>> Data; // vector to account for array-type resources, size=1 otherwise
		VulkanShaderResourceMetadata Metadata;
		std::vector<VkDescriptorImageInfo> ImageInfos;
		std::vector<VkDescriptorBufferInfo> BufferInfos;
		std::vector<VkBufferView> TexelBufferViews;
		bool Provided = false;
	};	

	class VulkanShaderResourceManager : public ShaderResourceManager
	{
	public:
		VulkanShaderResourceManager(const ShaderResourceManagerSpecification& specification);
		~VulkanShaderResourceManager();

		virtual void ProvideResource(const std::string& name, Ref<UniformBufferSet> uniformBufferSet) override;
		virtual void ProvideResource(const std::string& name, Ref<Texture2D> texture) override;
		virtual void ProvideResource(const std::string& name, const std::vector<Ref<Texture2D>>& textureArray) override;

		virtual bool CheckIfComplete() override;
		virtual void Update() override;

		std::vector<VkDescriptorSet>& GetDescriptorSets();
		std::vector<VkDescriptorSet>& GetDescriptorSets(uint32_t frame) { return m_DescriptorSets[frame]; }

		uint32_t GetFirstSet() { return m_FirstSet; }
		uint32_t GetLastSet() { return m_LastSet; }

	private:
		Ref<VulkanShader> m_Shader;
		uint32_t m_FirstSet, m_LastSet;

		// resources organised by corresponding name in shader code
		std::map<std::string, VulkanShaderResource> m_Resources;
		
		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;

		// descriptors organised by (frame-in-flight, set)
		std::vector<std::vector<VkDescriptorSet>> m_DescriptorSets;
		std::vector<VkWriteDescriptorSet> m_UpdateQueue;

		void Init();
		void CreateDescriptorPool();
		void AllocateDescriptorSets();
		//void PopulateWriteStructs();
	};

}
