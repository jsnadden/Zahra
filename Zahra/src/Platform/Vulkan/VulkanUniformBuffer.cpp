#include "zpch.h"
#include "VulkanUniformBuffer.h"

#include "Platform/Vulkan/VulkanContext.h"

namespace Zahra
{
	VulkanUniformBuffer::VulkanUniformBuffer(uint64_t size)
	{
		Init(size);
	}

	VulkanUniformBuffer::VulkanUniformBuffer(const void* data, uint64_t size, uint64_t offset)
	{
		Init(size);
		SetData(data, size, offset);
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		VkDevice& device = VulkanContext::GetCurrentDevice()->LogicalDevice;

		vkDeviceWaitIdle(device);

		vkDestroyBuffer(device, m_VulkanBuffer, nullptr);
		vkFreeMemory(device, m_VulkanBufferMemory, nullptr);

		m_Data.Release();
	}

	void VulkanUniformBuffer::SetData(const void* data, uint64_t size, uint64_t offset)
	{
		m_Data.Write(data, size, offset);

		memcpy(m_MappedAddress, m_Data.GetData<void>(), size);
	}

	void VulkanUniformBuffer::Init(uint64_t size)
	{
		m_Data.Allocate(size);
		m_Data.ZeroInitialise();

		Ref<VulkanDevice> device = VulkanContext::GetCurrentDevice();

		device->CreateVulkanBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_VulkanBuffer, m_VulkanBufferMemory);

		vkMapMemory(device->LogicalDevice, m_VulkanBufferMemory, 0, size, 0, &m_MappedAddress);
	}

}
