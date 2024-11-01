#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace Zahra
{
	enum class GPUQueueType
	{
		Graphics,
		Present,
		Transfer,
		Compute
	};

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> GraphicsIndex;
		std::optional<uint32_t> PresentIndex;
		std::optional<uint32_t> TransferIndex;
		std::optional<uint32_t> ComputeIndex;

		QueueFamilyIndices() = default;
		QueueFamilyIndices(const QueueFamilyIndices&) = default;

		bool Complete()
		{
			// TODO: more robust checking
			return GraphicsIndex.has_value() && PresentIndex.has_value();
		}

		const std::optional<uint32_t>& GetIndex(GPUQueueType queueType)
		{
			switch (queueType)
			{
				case Zahra::GPUQueueType::Graphics:
					return GraphicsIndex;
				case Zahra::GPUQueueType::Present:
					return PresentIndex;
				case Zahra::GPUQueueType::Transfer:
					return TransferIndex;
				case Zahra::GPUQueueType::Compute:
					return ComputeIndex;
				default:
					break;
			}

			Z_CORE_ASSERT(false, "Unrecognised GPUQueueType");
			return std::optional<uint32_t>();
		}

	};

	struct VulkanDeviceSwapchainSupport
	{
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentationModes;
		VkSurfaceCapabilitiesKHR Capabilities;
	};

	struct VulkanDevice : public RefCounted
	{
		VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		VkDevice LogicalDevice = VK_NULL_HANDLE;

		VkQueue GraphicsQueue = VK_NULL_HANDLE;
		VkQueue PresentationQueue = VK_NULL_HANDLE;
		VkQueue TransferQueue = VK_NULL_HANDLE;
		VkQueue ComputeQueue = VK_NULL_HANDLE;

		QueueFamilyIndices QueueFamilyIndices;
		VkPhysicalDeviceFeatures Features;
		VkPhysicalDeviceProperties Properties;
		VkPhysicalDeviceMemoryProperties Memory;
		VulkanDeviceSwapchainSupport SwapchainSupport;

		std::map<std::thread::id, VkCommandPool> m_CommandPools;

		void Shutdown();

		void CreateVulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags flags);

		void CopyVulkanBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		VkCommandBuffer GetTemporaryCommandBuffer();
		void FreeTemporaryCommandBuffer(VkCommandBuffer commandBuffer);
		VkCommandPool GetOrCreateCommandPool();

	};
	
}
