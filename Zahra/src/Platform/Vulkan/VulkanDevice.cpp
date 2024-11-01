#include "zpch.h"
#include "VulkanDevice.h"

#include "Platform/Vulkan/VulkanUtils.h"

namespace Zahra
{
	void VulkanDevice::Shutdown()
	{
		for (auto& [threadID, pool] : m_CommandPools)
			vkDestroyCommandPool(LogicalDevice, pool, nullptr);

		vkDestroyDevice(LogicalDevice, nullptr);
		LogicalDevice = VK_NULL_HANDLE;
	}

	void VulkanDevice::CreateVulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VulkanUtils::ValidateVkResult(vkCreateBuffer(LogicalDevice, &bufferInfo, nullptr, &buffer),
			"Vulkan buffer creation failed");

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(LogicalDevice, buffer, &memoryRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memoryRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, properties);

		VulkanUtils::ValidateVkResult(vkAllocateMemory(LogicalDevice, &allocInfo, nullptr, &bufferMemory),
			"Vulkan failed to allocate buffer memory");

		vkBindBufferMemory(LogicalDevice, buffer, bufferMemory, 0);
	}

	uint32_t VulkanDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags flags)
	{
		for (uint32_t i = 0; i < Memory.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && ((Memory.memoryTypes[i].propertyFlags & flags) == flags))
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

		vkQueueSubmit(GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(GraphicsQueue); // TODO: synchronise using fences instead, so we can queue up a bunch of transfers

		FreeTemporaryCommandBuffer(commandBuffer);
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
		vkAllocateCommandBuffers(LogicalDevice, &allocInfo, &commandBuffer);

		return commandBuffer;
	}

	void VulkanDevice::FreeTemporaryCommandBuffer(VkCommandBuffer commandBuffer)
	{
		VkCommandPool commandPool = GetOrCreateCommandPool();
		vkFreeCommandBuffers(LogicalDevice, commandPool, 1, &commandBuffer);
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
		commandPoolInfo.queueFamilyIndex = QueueFamilyIndices.GraphicsIndex.value();

		VulkanUtils::ValidateVkResult(vkCreateCommandPool(LogicalDevice, &commandPoolInfo, nullptr, &newCommandPool),
			"Vulkan command pool creation failed");

		m_CommandPools[threadID] = newCommandPool;

		return newCommandPool;
	}

}
