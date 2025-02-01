#include "zpch.h"
#include "VulkanShaderResourceManager.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanShaderUtils.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Zahra/Renderer/Renderer.h"

namespace Zahra
{

	VulkanShaderResourceManager::VulkanShaderResourceManager(const ShaderResourceManagerSpecification& specification)
		: m_Shader(specification.Shader.As<VulkanShader>()), m_FirstSet(specification.FirstSet), m_LastSet(specification.LastSet)
	{
		Init();
	}

	void VulkanShaderResourceManager::Init()
	{
		m_Resources.clear();
		m_DescriptorSets.clear();
		m_UpdateQueue.clear();

		const auto& reflectionData = m_Shader->m_ReflectionData;

		for (const auto& resource : reflectionData.ResourceMetadata)
		{
			if (resource.Set < m_FirstSet || resource.Set > m_LastSet)
				continue;

			auto& newShaderResource = m_Resources[resource.Name];
			newShaderResource.Metadata = resource;
			//newShaderResource.Data.resize(resource.ArrayLength);

			// for array-type resources, it's helpful to cache a vector of image/buffer infos
			if (resource.ArrayLength > 1)
			{
				switch (resource.Type)
				{
					case ShaderResourceType::Texture2D:
					{
						newShaderResource.ImageInfos.resize(resource.ArrayLength);
						break;
					}
					default:
					{
						Z_CORE_ASSERT(false, "This array type is not yet supported");
					}
				}
			}

			newShaderResource.HasBeenSet = false;
		}

		CreateDescriptorPool();
		AllocateDescriptorSets();
	}

	void VulkanShaderResourceManager::CreateDescriptorPool()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();
		uint32_t maxSetIndex = m_Shader->m_ReflectionData.MaxSetIndex;
		uint32_t framesInFlight = Renderer::GetFramesInFlight();

		if (m_DescriptorPool != VK_NULL_HANDLE)
			vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);

		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,			1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	1000 }
		}; // TODO: too many? too few? maybe should compute pool size from the metadata input

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = (maxSetIndex + 1) * framesInFlight;

		VulkanUtils::ValidateVkResult(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool),
			"Vulkan descriptor pool creation failed");
	}

	void VulkanShaderResourceManager::AllocateDescriptorSets()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();
		uint32_t maxSetIndex = m_Shader->m_ReflectionData.MaxSetIndex;
		uint32_t framesInFlight = Renderer::GetFramesInFlight();
		const std::vector<VkDescriptorSetLayout>& descriptorLayouts = m_Shader->m_DescriptorSetLayouts;

		m_DescriptorSets.resize(framesInFlight);
		for (auto& vec : m_DescriptorSets)
			vec.resize(maxSetIndex + 1);

		for (uint32_t set = m_FirstSet; set <= m_LastSet; set++)
		{
			std::vector<VkDescriptorSetLayout> layouts(framesInFlight, descriptorLayouts[set]);

			VkDescriptorSetAllocateInfo descriptorSetAllocationInfo{};
			descriptorSetAllocationInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocationInfo.descriptorPool = m_DescriptorPool;
			descriptorSetAllocationInfo.descriptorSetCount = framesInFlight;
			descriptorSetAllocationInfo.pSetLayouts = layouts.data();

			std::vector<VkDescriptorSet> descriptorPerFrame(framesInFlight);

			VulkanUtils::ValidateVkResult(vkAllocateDescriptorSets(device, &descriptorSetAllocationInfo, descriptorPerFrame.data()),
				"Descriptor set allocation failed");

			for (uint32_t frame = 0; frame < framesInFlight; frame++)
			{
				m_DescriptorSets[frame][set] = descriptorPerFrame[frame];
			}
		}
	}

	VulkanShaderResourceManager::~VulkanShaderResourceManager()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();
		vkDeviceWaitIdle(device);
		vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
	}

	void VulkanShaderResourceManager::Set(const std::string& name, Ref<UniformBufferPerFrame> uniformBufferPerFrame)
	{
		auto& search = m_Resources.find(name);
		Z_CORE_ASSERT(search != m_Resources.end(), "No expected resource was found with the provided name");

		VulkanShaderResource& resource = search->second;
		Z_CORE_ASSERT(resource.Metadata.Type == ShaderResourceType::UniformBuffer, "Wrong resource type provided");
		Z_CORE_ASSERT(resource.Metadata.ArrayLength == 1, "This method is not intended for use with array-type resources");

		//resource.Data[0] = uniformBufferPerFrame;

		for (uint32_t frame = 0; frame < Renderer::GetFramesInFlight(); frame++)
		{
			auto& write = m_UpdateQueue.emplace_back();
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = m_DescriptorSets[frame][resource.Metadata.Set];
			write.dstBinding = resource.Metadata.Binding;
			write.dstArrayElement = 0;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.pImageInfo = nullptr;
			write.pBufferInfo = &uniformBufferPerFrame->Get(frame).As<VulkanUniformBuffer>()->GetVkDescriptorBufferInfo();
			write.pTexelBufferView = nullptr;
		}

		resource.HasBeenSet = true;
	}

	void VulkanShaderResourceManager::Set(const std::string& name, Ref<Texture2D> texture)
	{
		auto& search = m_Resources.find(name);
		Z_CORE_ASSERT(search != m_Resources.end(), "No expected resource was found with the provided name");

		VulkanShaderResource& resource = search->second;
		Z_CORE_ASSERT(resource.Metadata.Type == ShaderResourceType::Texture2D, "Wrong resource type provided");
		Z_CORE_ASSERT(resource.Metadata.ArrayLength == 1, "This method is not intended for use with array-type resources");

		//resource.Data[0] = texture;

		for (uint32_t frame = 0; frame < Renderer::GetFramesInFlight(); frame++)
		{
			auto& write = m_UpdateQueue.emplace_back();
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = m_DescriptorSets[frame][resource.Metadata.Set];
			write.dstBinding = resource.Metadata.Binding;
			write.dstArrayElement = 0;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.pImageInfo = &texture.As<VulkanTexture2D>()->GetVkDescriptorImageInfo();
			write.pBufferInfo = nullptr;
			write.pTexelBufferView = nullptr;
		}

		resource.HasBeenSet = true;
	}

	void VulkanShaderResourceManager::Set(const std::string& name, const std::vector<Ref<Texture2D>>& textureArray)
	{
		auto& search = m_Resources.find(name);
		Z_CORE_ASSERT(search != m_Resources.end(), "No expected resource was found with the provided name");

		VulkanShaderResource& resource = search->second;
		Z_CORE_ASSERT(resource.Metadata.Type == ShaderResourceType::Texture2D, "Wrong resource type provided");
		Z_CORE_ASSERT(resource.Metadata.ArrayLength == textureArray.size(), "Provided array doesn't have the correct size");

		for (uint32_t i = 0; i < resource.Metadata.ArrayLength; i++)
		{
			//resource.Data[i] = textureArray[i];
			resource.ImageInfos[i] = textureArray[i].As<VulkanTexture2D>()->GetVkDescriptorImageInfo();
		}

		for (uint32_t frame = 0; frame < Renderer::GetFramesInFlight(); frame++)
		{
			auto& write = m_UpdateQueue.emplace_back();
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = m_DescriptorSets[frame][resource.Metadata.Set];
			write.dstBinding = resource.Metadata.Binding;
			write.dstArrayElement = 0;
			write.descriptorCount = (uint32_t)resource.ImageInfos.size();
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.pImageInfo = resource.ImageInfos.data();
			write.pBufferInfo = nullptr;
			write.pTexelBufferView = nullptr;
		}

		resource.HasBeenSet = true;
	}

	void VulkanShaderResourceManager::Update(const std::string& name, Ref<UniformBuffer> uniformBuffer)
	{
		auto& search = m_Resources.find(name);
		Z_CORE_ASSERT(search != m_Resources.end(), "No expected resource was found with the provided name");
		
		VulkanShaderResource& resource = search->second;
		Z_CORE_ASSERT(resource.Metadata.Type == ShaderResourceType::UniformBuffer, "Wrong resource type provided");
		Z_CORE_ASSERT(resource.Metadata.ArrayLength == 1, "This method is not intended for use with array-type resources");

		uint32_t frame = Renderer::GetCurrentFrameIndex();
		
		auto& write = m_UpdateQueue.emplace_back();
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = m_DescriptorSets[frame][resource.Metadata.Set];
		write.dstBinding = resource.Metadata.Binding;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.pImageInfo = nullptr;
		write.pBufferInfo = &uniformBuffer.As<VulkanUniformBuffer>()->GetVkDescriptorBufferInfo();
		write.pTexelBufferView = nullptr;

		resource.HasBeenSet = true;
	}

	void VulkanShaderResourceManager::Update(const std::string& name, Ref<Texture2D> texture)
	{
		auto& search = m_Resources.find(name);
		Z_CORE_ASSERT(search != m_Resources.end(), "No expected resource was found with the provided name");

		VulkanShaderResource& resource = search->second;
		Z_CORE_ASSERT(resource.Metadata.Type == ShaderResourceType::Texture2D, "Wrong resource type provided");
		Z_CORE_ASSERT(resource.Metadata.ArrayLength == 1, "This method is not intended for use with array-type resources");

		uint32_t frame = Renderer::GetCurrentFrameIndex();

		auto& write = m_UpdateQueue.emplace_back();
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = m_DescriptorSets[frame][resource.Metadata.Set];
		write.dstBinding = resource.Metadata.Binding;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &texture.As<VulkanTexture2D>()->GetVkDescriptorImageInfo();
		write.pBufferInfo = nullptr;
		write.pTexelBufferView = nullptr;

		resource.HasBeenSet = true;
	}

	void VulkanShaderResourceManager::Update(const std::string& name, const std::vector<Ref<Texture2D>>& textureArray)
	{
		auto& search = m_Resources.find(name);
		Z_CORE_ASSERT(search != m_Resources.end(), "No expected resource was found with the provided name");

		VulkanShaderResource& resource = search->second;
		Z_CORE_ASSERT(resource.Metadata.Type == ShaderResourceType::Texture2D, "Wrong resource type provided");
		Z_CORE_ASSERT(resource.Metadata.ArrayLength == textureArray.size(), "Provided array doesn't have the correct size");

		for (uint32_t i = 0; i < resource.Metadata.ArrayLength; i++)
		{
			//resource.Data[i] = textureArray[i];
			resource.ImageInfos[i] = textureArray[i].As<VulkanTexture2D>()->GetVkDescriptorImageInfo();
		}

		uint32_t frame = Renderer::GetCurrentFrameIndex();

		auto& write = m_UpdateQueue.emplace_back();
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = m_DescriptorSets[frame][resource.Metadata.Set];
		write.dstBinding = resource.Metadata.Binding;
		write.dstArrayElement = 0;
		write.descriptorCount = (uint32_t)resource.ImageInfos.size();
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = resource.ImageInfos.data();
		write.pBufferInfo = nullptr;
		write.pTexelBufferView = nullptr;

		resource.HasBeenSet = true;
	}

	bool VulkanShaderResourceManager::ReadyToRender()
	{
		for (auto [name, resource] : m_Resources)
		{
			if (!resource.HasBeenSet)
				return false;
		}

		return true;
	}

	void VulkanShaderResourceManager::ProcessChanges()
	{
		vkUpdateDescriptorSets(VulkanContext::GetCurrentVkDevice(), (uint32_t)m_UpdateQueue.size(), m_UpdateQueue.data(), 0, nullptr);
		m_UpdateQueue.clear();
	}

	std::vector<VkDescriptorSet>& VulkanShaderResourceManager::GetDescriptorSets()
	{
		return GetDescriptorSets(Renderer::GetCurrentFrameIndex());
	}

	

}
