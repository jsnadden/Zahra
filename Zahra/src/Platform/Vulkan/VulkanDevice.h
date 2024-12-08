#pragma once

#include "Zahra/Core/Ref.h"
#include "Zahra/Core/Assert.h"

#include <optional>
#include <vector>
#include <map>
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

	class VulkanDevice : public RefCounted
	{
	public:
		VulkanDevice() = default;
		~VulkanDevice();

		void Shutdown();

		void CreateVulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyVulkanBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);
		
		// TODO: expand these to include non-2D images (and other options)
		void CreateVulkanImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		VkImageView CreateVulkanImageView(VkFormat format, VkImage& image, VkImageAspectFlags aspectFlags);
		VkSampler CreateVulkanImageSampler(VkFilter minFilter, VkFilter magFilter, VkSamplerAddressMode tilingMode);
		void CopyVulkanBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void CopyVulkanImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);
		void TransitionVulkanImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		VkCommandBuffer GetTemporaryCommandBuffer(bool begin = true);
		void EndTemporaryCommandBuffer(VkCommandBuffer commandBuffer);

		VkCommandBuffer GetSecondaryCommandBuffer();

		VkPhysicalDevice& GetPhysicalDevice() { return m_PhysicalDevice; }
		VkDevice& GetVkDevice() { return m_LogicalDevice; }
		QueueFamilyIndices& GetQueueFamilyIndices() { return m_QueueFamilyIndices; }
		VkQueue& GetGraphicsQueue() { return m_GraphicsQueue; }

		VkFormat CheckFormatSupport(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);


	private:
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_LogicalDevice = VK_NULL_HANDLE;

		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentationQueue = VK_NULL_HANDLE;
		VkQueue m_TransferQueue = VK_NULL_HANDLE;
		VkQueue m_ComputeQueue = VK_NULL_HANDLE;

		QueueFamilyIndices m_QueueFamilyIndices;
		VkPhysicalDeviceFeatures m_Features;
		VkPhysicalDeviceProperties m_Properties;
		VkPhysicalDeviceMemoryProperties m_MemoryProperties;
		VulkanDeviceSwapchainSupport m_SwapchainSupport;

		std::map<std::thread::id, VkCommandPool> m_CommandPools;

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags flags);

		VkCommandPool GetOrCreateCommandPool();

		friend class VulkanSwapchain;
		friend class VulkanContext;
	};
	
}
