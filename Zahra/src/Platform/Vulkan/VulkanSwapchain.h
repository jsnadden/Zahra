#pragma once

#include "Platform/Vulkan/VulkanDevice.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	class VulkanSwapchain
	{
	public:
		VulkanSwapchain() = default;

		void Init(Ref<VulkanDevice> device, VkSurfaceKHR surface);
		void Shutdown();

	private:
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;

		VkSurfaceFormatKHR m_Format;
		VkPresentModeKHR m_PresentationMode;
		VkExtent2D m_Extent;

		Ref<VulkanDevice> m_Device;
		VkSurfaceKHR m_Surface;

		VkSurfaceFormatKHR ChooseSwapchainFormat();
		VkPresentModeKHR ChooseSwapchainPresentationMode();
		VkExtent2D ChooseSwapchainExtent();

		friend class VulkanContext;
	};

}
