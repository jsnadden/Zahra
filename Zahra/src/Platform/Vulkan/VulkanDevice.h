#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace Zahra
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> GraphicsIndex;
		std::optional<uint32_t> PresentationIndex;
		std::optional<uint32_t> TransferIndex;
		std::optional<uint32_t> ComputeIndex;

		QueueFamilyIndices() = default;
		QueueFamilyIndices(const QueueFamilyIndices&) = default;

		bool Complete()
		{
			// TODO: more robust checking
			return GraphicsIndex.has_value() && PresentationIndex.has_value();
		}

	};

	struct VulkanDeviceSwapchainSupport
	{
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentationModes;
		VkSurfaceCapabilitiesKHR Capabilities;
	};

	struct VulkanDevice
	{
		VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		VkDevice Device = VK_NULL_HANDLE;

		VkQueue GraphicsQueue = VK_NULL_HANDLE;
		VkQueue PresentationQueue = VK_NULL_HANDLE;
		VkQueue TransferQueue = VK_NULL_HANDLE;
		VkQueue ComputeQueue = VK_NULL_HANDLE;

		QueueFamilyIndices QueueFamilyIndices;
		VkPhysicalDeviceFeatures Features;
		VkPhysicalDeviceProperties Properties;
		VkPhysicalDeviceMemoryProperties Memory;
		VulkanDeviceSwapchainSupport SwapchainSupport;
	};
	
}
