#include "zpch.h"
#include "VulkanVertexBuffer.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUtils.h"

namespace Zahra
{

	VulkanVertexBuffer::VulkanVertexBuffer(uint64_t size)
	{
		Init(size);
	}

	VulkanVertexBuffer::VulkanVertexBuffer(const void* data, uint64_t size)
	{
		Init(size);
		SetData(data, size);
	}
	
	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();

		vkDeviceWaitIdle(device);

		vkDestroyBuffer(device, m_VulkanVertexBuffer, nullptr);
		vkFreeMemory(device, m_VulkanVertexBufferMemory, nullptr);

		// TODO: decide whether local data should be released based on RendererConfig?
		//m_VertexData.Release();
	}
	
	void VulkanVertexBuffer::SetData(const void* data, uint64_t size)
	{
		m_VertexData.Write(data, size, 0);

		VkDevice& device = VulkanContext::GetCurrentVkDevice();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		VulkanContext::GetCurrentDevice()->CreateVulkanBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* mappedAddress;
		vkMapMemory(device, stagingBufferMemory, 0, size, 0, &mappedAddress);
		memcpy(mappedAddress, m_VertexData.GetData<void>(), size);
		vkUnmapMemory(device, stagingBufferMemory);

		VulkanContext::GetCurrentDevice()->CopyVulkanBuffer(stagingBuffer, m_VulkanVertexBuffer, size);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

		m_VertexData.Release();

	}

	void VulkanVertexBuffer::Init(uint64_t size)
	{
		m_VertexData.Allocate(size);
		m_VertexData.ZeroInitialise();

		// TODO: for optimal dynamic memory allocation, I should rewrite this class to use the
		// VulkanMemoryAllocator library (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
		// in place of VkAllocateMemory calls, which is fairly limited

		VulkanContext::GetCurrentDevice()->CreateVulkanBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VulkanVertexBuffer, m_VulkanVertexBufferMemory);
	}


}
