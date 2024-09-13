#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace Zahra
{
	struct VulkanSwapchain
	{
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentationModes;
	};
}
