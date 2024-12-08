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

	VulkanShaderResourceManager::~VulkanShaderResourceManager()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();
		vkDeviceWaitIdle(device);
		vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
	}

	void VulkanShaderResourceManager::ProvideResource(const std::string& name, Ref<UniformBufferSet> uniformBufferSet, uint32_t arrayIndex)
	{
		auto it = m_Resources.find(name);
		Z_CORE_ASSERT(it != m_Resources.end(), "No expected resource was found with the provided name");
		Z_CORE_ASSERT(m_Resources[name].Metadata.Type == ShaderResourceType::UniformBuffer, "Wrong resource type provided");

		m_Resources.at(name).Data[arrayIndex] = uniformBufferSet;
	}

	void VulkanShaderResourceManager::ProvideResource(const std::string& name, Ref<Texture2D> texture, uint32_t arrayIndex)
	{
		auto it = m_Resources.find(name);
		Z_CORE_ASSERT(it != m_Resources.end(), "No expected resource was found with the provided name");
		Z_CORE_ASSERT(m_Resources[name].Metadata.Type == ShaderResourceType::Texture2D, "Wrong resource type provided");

		m_Resources.at(name).Data[arrayIndex] = texture;
	}

	bool VulkanShaderResourceManager::CheckIfComplete()
	{
		// TODO: check if all managed resources have been correctly submitted
		return true;
	}

	void VulkanShaderResourceManager::Bake()
	{
		if (!CheckIfComplete()) throw std::runtime_error("VulkanShaderResourceManager is missing required resources");

		uint32_t framesInFlight = Renderer::GetFramesInFlight();
		uint32_t maxSetIndex = m_Shader->m_ReflectionData.MaxSetIndex;
		VkDevice& device = VulkanContext::GetCurrentVkDevice();
		const std::vector<VkDescriptorSetLayout>& descriptorLayouts = m_Shader->m_DescriptorSetLayouts;

		//////////////////////////////////////////////////////////////////////////////////////////////////////
		// CREATE DESCRIPTOR POOL
		if(m_DescriptorPool != VK_NULL_HANDLE)
			vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);

		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,			1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	1000 }
		}; // TODO: too many? too few? maybe should compute pool size from the metadata input

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = (maxSetIndex + 1) * framesInFlight;

		VulkanUtils::ValidateVkResult(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool),
			"Vulkan descriptor pool creation failed");

		//////////////////////////////////////////////////////////////////////////////////////////////////////
		// ALLOCATE DESCRIPTOR SETS
		m_DescriptorSets.resize(framesInFlight);
		for (auto& vec : m_DescriptorSets) vec.resize(maxSetIndex + 1);

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

		//////////////////////////////////////////////////////////////////////////////////////////////////////
		// UPDATE DESCRIPTOR SETS
		for (uint32_t frame = 0; frame < framesInFlight; frame++)
		{
			for (auto&& [name, resource] : m_Resources)
			{
				VkWriteDescriptorSet write{};
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = m_DescriptorSets[frame][resource.Metadata.Set];
				write.dstBinding = resource.Metadata.Binding;
				write.descriptorType = VulkanUtils::ShaderResourceTypeToVkDescriptorType(resource.Metadata.Type);
				write.dstArrayElement = 0;
				write.descriptorCount = resource.Metadata.ArrayLength;
				
				std::vector<VkDescriptorBufferInfo> bufferInfos;
				std::vector<VkDescriptorImageInfo> imageInfos;
				std::vector<VkBufferView> texelBufferViews;

				switch (resource.Metadata.Type)
				{
					case ShaderResourceType::UniformBuffer:
					{
						for (uint32_t i = 0; i < resource.Metadata.ArrayLength; i++)
						{
							bufferInfos.emplace_back(resource.Data[i].As<VulkanUniformBufferSet>()->Get(frame).As<VulkanUniformBuffer>()->GetVkDescriptorBufferInfo());
						}

						write.pBufferInfo = bufferInfos.data();
						break;
					}

					case ShaderResourceType::Texture2D:
					{
						for (uint32_t i = 0; i < resource.Metadata.ArrayLength; i++)
						{
							imageInfos.emplace_back(resource.Data[i].As<VulkanTexture2D>()->GetVkDescriptorImageInfo());
						}

						write.pImageInfo = imageInfos.data();
						break;

					}

					// TODO: other types!

					default: break;
				}

				// TODO: refactor to queue all these writes up in a single array and submit them together
				vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

			}
		}

	}

	std::vector<VkDescriptorSet>& VulkanShaderResourceManager::GetDescriptorSets()
	{
		return GetDescriptorSets(Renderer::GetCurrentFrameIndex());
	}

	void VulkanShaderResourceManager::Init()
	{
		const auto& reflectionData = m_Shader->m_ReflectionData;

		for (const auto& resource : reflectionData.ResourceMetadata)
		{
			if (resource.Set < m_FirstSet || resource.Set > m_LastSet)
				continue;

			m_Resources[resource.Name].Metadata = resource;
			m_Resources[resource.Name].Data.resize(resource.ArrayLength);
		}

	}


}
