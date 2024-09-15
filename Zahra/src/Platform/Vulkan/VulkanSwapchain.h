#pragma once

#include <vulkan/vulkan.h>

namespace Zahra
{
	struct VulkanSwapchain
	{
		VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
		std::vector<VkImage> Images;

		VkSurfaceFormatKHR Format;
		VkPresentModeKHR PresentationMode;
		VkExtent2D Extent;

		VulkanSwapchain() = default;
	};

}
