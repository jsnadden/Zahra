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
		void Invalidate();
		void Shutdown(VkInstance& instance);
		void Cleanup();

		void SignalResize();
		bool Invalidated() { return m_Invalidated; }

		void GetNextImage();
		void ExecuteDrawCommandBuffer();
		void PresentImage();

		VkSurfaceKHR& GetSurface() { return m_Surface; }

		Ref<VulkanDevice> GetDevice() { return m_Device; }
		const VkDevice& GetVkDevice() { return m_Device->m_LogicalDevice; }

		const VkExtent2D& GetExtent() { return m_Extent; }
		const VkFormat& GetSwapchainImageFormat() { return m_SurfaceFormat.format; }
		const std::vector<VkImageView>& GetSwapchainImageViews() { return m_ImageViews; }

		VkCommandBuffer GetDrawCommandBuffer(uint32_t index);
		VkCommandBuffer GetCurrentDrawCommandBuffer() { return GetDrawCommandBuffer(m_CurrentFrameIndex); }

		const uint32_t GetFramesInFlight() const { return m_FramesInFlight; }
		const uint32_t GetImageCount() const { return m_ImageCount; }
		const uint32_t GetFrameIndex() const { return m_CurrentFrameIndex; }
		const uint32_t GetImageIndex() const { return m_CurrentImageIndex; }

	private:
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

		bool m_WindowResized = false;
		bool m_Invalidated = false;

		uint32_t m_ImageCount = 0;
		uint32_t m_CurrentImageIndex = 0;
		std::vector<VkImage> m_Images;
		VkFormat m_ImageFormat;
		std::vector<VkImageView> m_ImageViews;

		uint32_t m_FramesInFlight = 3;
		uint32_t m_CurrentFrameIndex = 0;

		Ref<VulkanDevice> m_Device;

		VkSurfaceFormatKHR m_SurfaceFormat;
		VkPresentModeKHR m_PresentationMode;
		VkExtent2D m_Extent;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_DrawCommandBuffers;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;

		void CreateSurface(VkInstance& instance, GLFWwindow* windowHandle);

		// TODO: move this logic into the VulkanDevice class itself
		void CreateDevice(VkInstance& instance);
		void TargetPhysicalDevice(VkInstance& instance);
		bool MeetsMinimimumRequirements(const VkPhysicalDevice& device);
		bool CheckDeviceExtensionSupport(const VkPhysicalDevice& device, const std::vector<const char*>& extensions);
		void IdentifyQueueFamilies(const VkPhysicalDevice& device, QueueFamilyIndices& indices);
		bool CheckSwapchainSupport(const VkPhysicalDevice& device, VulkanDeviceSwapchainSupport& support);
		void QuerySurfaceCapabilities(const VkPhysicalDevice& device, VkSurfaceCapabilitiesKHR& capabilities);

		void CreateSwapchain();
		VkSurfaceFormatKHR ChooseSwapchainFormat();
		VkPresentModeKHR ChooseSwapchainPresentationMode();
		VkExtent2D ChooseSwapchainExtent();

		void GetSwapchainImagesAndCreateImageViews();

		void CreateCommandPool();
		void AllocateCommandBuffer();

		void CreateSyncObjects();

		friend class VulkanContext;
	};

}
