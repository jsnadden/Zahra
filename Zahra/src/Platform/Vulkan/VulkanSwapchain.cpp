#include "zpch.h"
#include "VulkanSwapchain.h"

#include "Platform/Vulkan/VulkanUtils.h"
#include "Zahra/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Zahra
{
	void VulkanSwapchain::Init(Ref<VulkanDevice> device, VkSurfaceKHR surface)
	{
		m_Device = device;
		m_Surface = surface;

		m_Format = ChooseSwapchainFormat();
		m_PresentationMode = ChooseSwapchainPresentationMode();
		m_Extent = ChooseSwapchainExtent();

		uint32_t swapchainLength = m_Device->SwapchainSupport.Capabilities.minImageCount + 1;
		uint32_t maxImageCount = m_Device->SwapchainSupport.Capabilities.maxImageCount;
		if (maxImageCount > 0 && swapchainLength > maxImageCount) swapchainLength = maxImageCount;

		VkSwapchainCreateInfoKHR swapchainInfo{};
		swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainInfo.surface = m_Surface;
		swapchainInfo.minImageCount = swapchainLength;
		swapchainInfo.imageFormat = m_Format.format;
		swapchainInfo.imageColorSpace = m_Format.colorSpace;
		swapchainInfo.presentMode = m_PresentationMode;
		swapchainInfo.imageExtent = m_Extent;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] =
		{
			m_Device->QueueFamilyIndices.GraphicsIndex.value(),
			m_Device->QueueFamilyIndices.PresentationIndex.value()
		};

		if (m_Device->QueueFamilyIndices.GraphicsIndex = m_Device->QueueFamilyIndices.PresentationIndex)
		{
			// EXCLUSIVE MODE: An image is owned by one queue family
			// at a time and ownership must be explicitly transferred
			// before using it in another queue family. This option
			// offers the best performance.
			swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainInfo.queueFamilyIndexCount = 0;
			swapchainInfo.pQueueFamilyIndices = nullptr;
		}
		else
		{
			// CONCURRENT MODE: Images can be used across multiple queue
			// families without explicit ownership transfers. This requires
			// you to specify in advance between which queue families
			// ownership will be shared using the queueFamilyIndexCount
			// and pQueueFamilyIndices parameters. Could also use exclusive
			// mode in this case, but it's more complicated.
			swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainInfo.queueFamilyIndexCount = 2;
			swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		swapchainInfo.preTransform = m_Device->SwapchainSupport.Capabilities.currentTransform; // no overall screen rotation/flip
		swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // no alpha blending with other windows
		swapchainInfo.clipped = VK_TRUE; // ignore pixels obscured by other windows

		// With Vulkan it's possible that your swap chain becomes invalid
		// or unoptimized while your application is running, for example
		// because the window was resized. In that case the swap chain
		// actually needs to be recreated from scratch and a reference to
		// the old one must be specified in this field.
		swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

		VulkanUtils::ValidateVkResult(vkCreateSwapchainKHR(m_Device->Device, &swapchainInfo, nullptr, &m_Swapchain), "Vulkan swap chain creation failed");

		vkGetSwapchainImagesKHR(m_Device->Device, m_Swapchain, &swapchainLength, nullptr);
		m_Images.resize(swapchainLength);
		vkGetSwapchainImagesKHR(m_Device->Device, m_Swapchain, &swapchainLength, m_Images.data());

		m_ImageViews.resize(swapchainLength);
		for (size_t i = 0; i < swapchainLength; i++)
		{
			VkImageViewCreateInfo imageViewInfo{};
			imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewInfo.image = m_Images[i];
			imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewInfo.format = m_Format.format;
			imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewInfo.subresourceRange.baseMipLevel = 0;
			imageViewInfo.subresourceRange.levelCount = 1;
			imageViewInfo.subresourceRange.baseArrayLayer = 0;
			imageViewInfo.subresourceRange.layerCount = 1;

			VulkanUtils::ValidateVkResult(vkCreateImageView(m_Device->Device, &imageViewInfo,
				nullptr, &m_ImageViews[i]), "Vulkan image view creation failed");
		}

		Z_CORE_INFO("Vulkan swap chain creation succeeded");
	}

	void VulkanSwapchain::Shutdown()
	{
		for (auto imageView : m_ImageViews)
		{
			vkDestroyImageView(m_Device->Device, imageView, nullptr);
		}
		m_ImageViews.clear();

		vkDestroySwapchainKHR(m_Device->Device, m_Swapchain, nullptr);

		m_Images.clear();
		m_Swapchain = VK_NULL_HANDLE;
	}

	VkSurfaceFormatKHR VulkanSwapchain::ChooseSwapchainFormat()
	{
		for (const auto& format : m_Device->SwapchainSupport.Formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}

		// TODO: rank formats and return best. For now just...
		return m_Device->SwapchainSupport.Formats[0];
	}

	VkPresentModeKHR VulkanSwapchain::ChooseSwapchainPresentationMode()
	{
		for (const auto& mode : m_Device->SwapchainSupport.PresentationModes)
		{
			// triple+ buffering
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return mode;
			}
		}

		// TODO: more detailed preferencing

		// standard double buffering
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanSwapchain::ChooseSwapchainExtent()
	{
		const auto& capabilities = m_Device->SwapchainSupport.Capabilities;

		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}

		GLFWwindow* windowHandle = Application::Get().GetWindow().GetWindowHandle();

		int width, height;
		glfwGetFramebufferSize(windowHandle, &width, &height);

		VkExtent2D extent = { (uint32_t)width, (uint32_t)height };

		extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return extent;
	}

}

