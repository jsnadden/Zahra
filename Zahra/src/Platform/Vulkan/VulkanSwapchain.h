#pragma once

#include "Platform/Vulkan/VulkanDevice.h"

#include <vulkan/vulkan.h>

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

		Ref<VulkanDevice> GetDevice() { return m_Device; }
		VkDevice& GetVkDevice() { return m_Device->m_LogicalDevice; }

		const VkExtent2D& GetExtent() { return m_Extent; }
		uint32_t GetWidth() { return m_Extent.width; }
		uint32_t GetHeight() { return m_Extent.height; }

		const VkFormat& GetSwapchainImageFormat() { return m_SurfaceFormat.format; }
		const std::vector<VkImageView>& GetSwapchainImageViews() { return m_ImageViews; }

		VkCommandBuffer& GetDrawCommandBuffer(uint32_t index);
		VkCommandBuffer& GetCurrentDrawCommandBuffer() { return GetDrawCommandBuffer(m_CurrentFrameIndex); }

		const uint32_t GetFramesInFlight() const { return m_FramesInFlight; }
		const uint32_t GetImageCount() const { return m_ImageCount; }
		const uint32_t GetFrameIndex() const { return m_CurrentFrameIndex; }
		const uint32_t GetImageIndex() const { return m_CurrentImageIndex; }

	private:
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

		Ref<VulkanDevice> m_Device;
		VulkanDeviceSwapchainSupport m_SwapchainSupport{};

		bool m_WindowResized = false;
		bool m_Invalidated = false;

		bool m_Initialised = false;

		uint32_t m_ImageCount = 0;
		uint32_t m_CurrentImageIndex = 0;
		std::vector<VkImage> m_Images;
		VkFormat m_ImageFormat;
		std::vector<VkImageView> m_ImageViews;

		uint32_t m_FramesInFlight = 3;
		uint32_t m_CurrentFrameIndex = 0;

		VkSurfaceFormatKHR m_SurfaceFormat;
		VkPresentModeKHR m_PresentationMode;
		VkExtent2D m_Extent;

		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_DrawCommandBuffers;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;

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
