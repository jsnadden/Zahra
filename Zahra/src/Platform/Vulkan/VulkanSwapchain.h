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

		void GetNextImage();
		void PresentImage();

		VkSurfaceKHR& GetSurface() { return m_Surface; }

		Ref<VulkanDevice> GetDevice() { return m_Device; }
		const VkDevice& GetLogicalDevice() { return m_Device->LogicalDevice; }

		const VkExtent2D& GetExtent() { return m_Extent; }
		const VkFormat& GetImageFormat() { return m_Format.format; }
		const std::vector<VkImageView>& GetImageViews() { return m_ImageViews; }

		const VkRenderPass& GetVkRenderPass() { return m_RenderPass; }

		VkFramebuffer GetFramebuffer(uint32_t index);
		VkFramebuffer GetCurrentFramebuffer() { return GetFramebuffer(m_CurrentImageIndex); }

		VkCommandBuffer GetDrawCommandBuffer(uint32_t index);
		VkCommandBuffer GetCurrentDrawCommandBuffer() { return GetDrawCommandBuffer(m_CurrentFrameIndex); }
		
	private:
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

		uint32_t m_ImageCount = 0;
		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;
		std::vector<VkFramebuffer> m_Framebuffers;
		uint32_t m_CurrentImageIndex = 0;

		uint32_t m_FramesInFlight = 3;
		uint32_t m_CurrentFrameIndex = 0;

		Ref<VulkanDevice> m_Device;

		VkSurfaceFormatKHR m_Format;
		VkPresentModeKHR m_PresentationMode;
		VkExtent2D m_Extent;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

		// TODO: does this belong here?
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;

		void CreateSurface(VkInstance& instance, GLFWwindow* windowHandle);

		void CreateDevice(VkInstance& instance);
		void TargetPhysicalDevice(VkInstance& instance);
		bool MeetsMinimimumRequirements(const VkPhysicalDevice& device);
		bool CheckDeviceExtensionSupport(const VkPhysicalDevice& device, const std::vector<const char*>& extensions);
		void IdentifyQueueFamilies(const VkPhysicalDevice& device, QueueFamilyIndices& indices);
		bool CheckSwapchainSupport(const VkPhysicalDevice& device, VulkanDeviceSwapchainSupport& support);

		void CreateSwapchain();
		VkSurfaceFormatKHR ChooseSwapchainFormat();
		VkPresentModeKHR ChooseSwapchainPresentationMode();
		VkExtent2D ChooseSwapchainExtent();

		void CreateImagesAndViews();
		void CreateRenderPass();
		void CreateFramebuffers();

		void CreateCommandPool();
		void AllocateCommandBuffer();

		void CreateSyncObjects();

		friend class VulkanContext;
	};

}
