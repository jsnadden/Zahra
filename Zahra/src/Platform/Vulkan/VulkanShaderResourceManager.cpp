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
		const auto& reflectionData = m_Shader->m_ReflectionData;

		for (const auto& resource : reflectionData.ResourceMetadata)
		{
			if (resource.Set < m_FirstSet || resource.Set > m_LastSet)
				continue;

			auto& newShaderResource = m_Resources[resource.Name];
			newShaderResource.Metadata = resource;
			newShaderResource.Data.resize(resource.ArrayLength);

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
					Z_CORE_ASSERT(false, "This array type is not yet supported");
				}
			}

			newShaderResource.Provided = false;
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

	void VulkanShaderResourceManager::ProvideResource(const std::string& name, Ref<UniformBufferSet> uniformBufferSet)
	{
		Z_CORE_ASSERT(m_Resources.find(name) != m_Resources.end(), "No expected resource was found with the provided name");

		VulkanShaderResource& resource = m_Resources[name];
		Z_CORE_ASSERT(resource.Metadata.Type == ShaderResourceType::UniformBuffer, "Wrong resource type provided");
		Z_CORE_ASSERT(resource.Metadata.ArrayLength == 1, "This method is not intended for use with array-type resources");

		resource.Data[0] = uniformBufferSet;

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
			write.pBufferInfo = &uniformBufferSet->Get(frame).As<VulkanUniformBuffer>()->GetVkDescriptorBufferInfo();
			write.pTexelBufferView = nullptr;
		}

		resource.Provided = true;
	}

	void VulkanShaderResourceManager::ProvideResource(const std::string& name, Ref<Texture2D> texture)
	{
		Z_CORE_ASSERT(m_Resources.find(name) != m_Resources.end(), "No expected resource was found with the provided name");

		VulkanShaderResource& resource = m_Resources[name];
		Z_CORE_ASSERT(resource.Metadata.Type == ShaderResourceType::Texture2D, "Wrong resource type provided");
		Z_CORE_ASSERT(resource.Metadata.ArrayLength == 1, "This method is not intended for use with array-type resources");

		resource.Data[0] = texture;

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

		resource.Provided = true;
	}

	void VulkanShaderResourceManager::ProvideResource(const std::string& name, const std::vector<Ref<Texture2D>>& textureArray)
	{
		Z_CORE_ASSERT(m_Resources.find(name) != m_Resources.end(), "No expected resource was found with the provided name");

		VulkanShaderResource& resource = m_Resources[name];
		Z_CORE_ASSERT(resource.Metadata.Type == ShaderResourceType::Texture2D, "Wrong resource type provided");
		Z_CORE_ASSERT(resource.Metadata.ArrayLength == textureArray.size(), "Provided array doesn't have the correct size");

		for (uint32_t i = 0; i < resource.Metadata.ArrayLength; i++)
		{
			resource.Data[i] = textureArray[i];
			resource.ImageInfos[i] = textureArray[i].As<VulkanTexture2D>()->GetVkDescriptorImageInfo();
		}

		for (uint32_t frame = 0; frame < Renderer::GetFramesInFlight(); frame++)
		{
			auto& write = m_UpdateQueue.emplace_back();
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = m_DescriptorSets[frame][resource.Metadata.Set];
			write.dstBinding = resource.Metadata.Binding;
			write.dstArrayElement = 0;
			write.descriptorCount = resource.ImageInfos.size();
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.pImageInfo = resource.ImageInfos.data();
			write.pBufferInfo = nullptr;
			write.pTexelBufferView = nullptr;
		}

		resource.Provided = true;
	}

	bool VulkanShaderResourceManager::CheckIfComplete()
	{
		for (auto [name, resource] : m_Resources)
		{
			if (!resource.Provided)
				return false;
		}

		return true;
	}

	void VulkanShaderResourceManager::Update()
	{
		if (!CheckIfComplete())
		{
			std::string errorMessage = "Resource manager for Shader '" + m_Shader->GetName() + "' is missing expected resources";
			throw std::runtime_error(errorMessage);
		}

		vkUpdateDescriptorSets(VulkanContext::GetCurrentVkDevice(), m_UpdateQueue.size(), m_UpdateQueue.data(), 0, nullptr);
		m_UpdateQueue.clear();
	}

	std::vector<VkDescriptorSet>& VulkanShaderResourceManager::GetDescriptorSets()
	{
		return GetDescriptorSets(Renderer::GetCurrentFrameIndex());
	}

	

}
