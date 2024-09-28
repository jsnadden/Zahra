#pragma once

#include "Platform/Vulkan/VulkanDevice.h"

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace Zahra
{
	class VulkanSwapchain : public RefCounted
	{
	public:
		VulkanSwapchain() = default;

		void Init(VkInstance& instance, GLFWwindow* windowHandle);
		void Shutdown(VkInstance& instance);

		VkSurfaceKHR& GetSurface() { return m_Surface; }
		Ref<VulkanDevice> GetDevice() { return m_Device; }
		const VkExtent2D& GetExtent() { return m_Extent; }
		const VkFormat& GetImageFormat() { return m_Format.format; }

	private:
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;

		VkSurfaceFormatKHR m_Format;
		VkPresentModeKHR m_PresentationMode;
		VkExtent2D m_Extent;

		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		Ref<VulkanDevice> m_Device;

		void CreateSurface(VkInstance& instance, GLFWwindow* windowHandle);

		void CreateDevice(VkInstance& instance);
		void ShutdownDevice();
		void TargetPhysicalDevice(VkInstance& instance);
		bool MeetsMinimimumRequirements(const VkPhysicalDevice& device);
		bool CheckDeviceExtensionSupport(const VkPhysicalDevice& device, const std::vector<const char*>& extensions);
		void IdentifyQueueFamilies(const VkPhysicalDevice& device, QueueFamilyIndices& indices);
		bool CheckSwapchainSupport(const VkPhysicalDevice& device, VulkanDeviceSwapchainSupport& support);

		VkSurfaceFormatKHR ChooseSwapchainFormat();
		VkPresentModeKHR ChooseSwapchainPresentationMode();
		VkExtent2D ChooseSwapchainExtent();

		friend class VulkanContext;
	};

}
