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
		//std::vector<Ref<RefCounted>> Data; // is a vector in case of resource arrays, size = 1 otherwise
		VulkanShaderResourceMetadata Metadata;
		std::vector<VkDescriptorImageInfo> ImageInfos;
		std::vector<VkDescriptorBufferInfo> BufferInfos;
		std::vector<VkBufferView> TexelBufferViews;
		bool HasBeenSet = false;
	};	

	class VulkanShaderResourceManager : public ShaderResourceManager
	{
	public:
		VulkanShaderResourceManager(const ShaderResourceManagerSpecification& specification);
		~VulkanShaderResourceManager();

		virtual void Set(const std::string& name, Ref<UniformBufferPerFrame> uniformBufferPerFrame) override;
		virtual void Set(const std::string& name, Ref<Texture2D> texture) override;
		virtual void Set(const std::string& name, const std::vector<Ref<Texture2D>>& textureArray) override;

		virtual void Update(const std::string& name, Ref<UniformBuffer> uniformBuffer) override;
		virtual void Update(const std::string& name, Ref<Texture2D> texture) override;
		virtual void Update(const std::string& name, const std::vector<Ref<Texture2D>>& textureArray) override;

		virtual bool ReadyToRender() override;
		virtual void ProcessChanges() override;

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
	};

}
