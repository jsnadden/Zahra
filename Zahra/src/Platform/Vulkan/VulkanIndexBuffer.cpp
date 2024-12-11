#include "zpch.h"
#include "VulkanIndexBuffer.h"

#include "Platform/Vulkan/VulkanContext.h"

namespace Zahra
{

	VulkanIndexBuffer::VulkanIndexBuffer(const uint32_t* indices, uint64_t count)
	{
		uint64_t size = count * sizeof(uint32_t);

		m_IndexData.Allocate(size);
		m_IndexData.ZeroInitialise();

		VulkanContext::GetCurrentDevice()->CreateVulkanBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VulkanIndexBuffer, m_VulkanIndexBufferMemory);

		SetData(indices, size);
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();

		vkDeviceWaitIdle(device);

		vkDestroyBuffer(device, m_VulkanIndexBuffer, nullptr);
		vkFreeMemory(device, m_VulkanIndexBufferMemory, nullptr);

		// TODO: decide whether local data should be released based on RendererConfig?
		//m_IndexData.Release();
	}

	void VulkanIndexBuffer::SetData(const uint32_t* data, uint64_t size)
	{
		m_IndexData.Write((void*)data, size, 0);

		m_IndexCount = size / sizeof(uint32_t);

		VkDevice& device = VulkanContext::GetCurrentVkDevice();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		VulkanContext::GetCurrentDevice()->CreateVulkanBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* mappedAddress;
		vkMapMemory(device, stagingBufferMemory, 0, size, 0, &mappedAddress);
		memcpy(mappedAddress, m_IndexData.GetData<void>(), size);
		vkUnmapMemory(device, stagingBufferMemory);

		VulkanContext::GetCurrentDevice()->CopyVulkanBuffer(stagingBuffer, m_VulkanIndexBuffer, size);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
		m_IndexData.Release();
	}

}
