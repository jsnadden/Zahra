#include "zpch.h"
#include "VulkanDevice.h"

#include "Platform/Vulkan/VulkanUtils.h"

namespace Zahra
{
	VulkanDevice::~VulkanDevice()
	{
		Shutdown();
	}

	void VulkanDevice::Shutdown()
	{
		for (auto& [threadID, pool] : m_CommandPools)
			vkDestroyCommandPool(m_LogicalDevice, pool, nullptr);
		m_CommandPools.clear();

		if (m_LogicalDevice) vkDestroyDevice(m_LogicalDevice, nullptr);
		m_LogicalDevice = VK_NULL_HANDLE;
	}

	void VulkanDevice::CreateVulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VulkanUtils::ValidateVkResult(vkCreateBuffer(m_LogicalDevice, &bufferInfo, nullptr, &buffer),
			"Vulkan buffer creation failed");

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(m_LogicalDevice, buffer, &memoryRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memoryRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, properties);

		VulkanUtils::ValidateVkResult(vkAllocateMemory(m_LogicalDevice, &allocInfo, nullptr, &bufferMemory),
			"Vulkan failed to allocate buffer memory");

		vkBindBufferMemory(m_LogicalDevice, buffer, bufferMemory, 0);
	}

	uint32_t VulkanDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags flags)
	{
		for (uint32_t i = 0; i < m_MemoryProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && ((m_MemoryProperties.memoryTypes[i].propertyFlags & flags) == flags))
				return i;
		}

		throw std::runtime_error("Vulkan device couldn't identify a suitable memory type");
	}

	void VulkanDevice::CopyVulkanBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = GetTemporaryCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_GraphicsQueue); // TODO: synchronise using fences instead, so we can queue up a bunch of transfers

		FreeTemporaryCommandBuffer(commandBuffer);
	}

	void VulkanDevice::CreateVulkanImage2D(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1; // TODO: expose this for mipmapping
		imageInfo.arrayLayers = 1; // I think this is for stereoscopic stuff?
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: expose this for multisampling (if using image as framebuffer attachment?)
		
		VulkanUtils::ValidateVkResult(vkCreateImage(m_LogicalDevice, &imageInfo, nullptr, &image),
			"Vulkan image creation failed");

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(m_LogicalDevice, image, &memoryRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memoryRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VulkanUtils::ValidateVkResult(vkAllocateMemory(m_LogicalDevice, &allocInfo, nullptr, &imageMemory),
			"Vulkan failed to allocate image memory");

		vkBindImageMemory(m_LogicalDevice, image, imageMemory, 0);

	}

	VkCommandBuffer VulkanDevice::GetTemporaryCommandBuffer()
	{
		VkCommandPool commandPool = GetOrCreateCommandPool();

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, &commandBuffer);

		return commandBuffer;
	}

	void VulkanDevice::FreeTemporaryCommandBuffer(VkCommandBuffer commandBuffer)
	{
		VkCommandPool commandPool = GetOrCreateCommandPool();
		vkFreeCommandBuffers(m_LogicalDevice, commandPool, 1, &commandBuffer);
	}

	VkCommandPool VulkanDevice::GetOrCreateCommandPool()
	{
		auto threadID = std::this_thread::get_id();
		auto pool = m_CommandPools.find(threadID);

		if (pool != m_CommandPools.end()) return pool->second;

		VkCommandPool newCommandPool;

		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		commandPoolInfo.queueFamilyIndex = m_QueueFamilyIndices.GraphicsIndex.value();

		VulkanUtils::ValidateVkResult(vkCreateCommandPool(m_LogicalDevice, &commandPoolInfo, nullptr, &newCommandPool),
			"Vulkan command pool creation failed");

		m_CommandPools[threadID] = newCommandPool;

		return newCommandPool;
	}

}
