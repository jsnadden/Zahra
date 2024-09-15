#pragma once

#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Zahra/Renderer/GraphicsContext.h"

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace Zahra
{
	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* handle);

		virtual void Init() override;
		virtual void Shutdown() override;
		virtual void SwapBuffers() override;

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
			void* userData)
		{
			// TODO: make this a more robust report, using all available data

			switch (messageSeverity)
			{
				case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				{
					Z_VULKAN_INFO(callbackData->pMessage);
					break;
				}
				case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				{
					Z_VULKAN_WARN(callbackData->pMessage);
					break;
				}
				case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				{
					Z_VULKAN_ERROR(callbackData->pMessage);
					break;
				}
				default:
					break;
			}

			return VK_FALSE;
		};
		
		const VkSurfaceKHR& GetSurface() { return m_Surface; }
		const Ref<VulkanDevice>& GetDevice() { return m_Device; }
		Ref<VulkanSwapchain> GetSwapchain() { return m_Swapchain; }

	private:
		
		GLFWwindow* m_WindowHandle;

		VkInstance m_VulkanInstance = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		Ref<VulkanDevice> m_Device;
		Ref<VulkanSwapchain> m_Swapchain;

		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

		void CreateInstance();
		void GetGLFWExtensions(std::vector<const char*>& extensions);
		bool CheckInstanceExtensionSupport(const std::vector<const char*>& extensions);
		bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers);

		void CreateDebugMessenger();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		void CreateSurface();

		void CreateDevice();
		void TargetPhysicalDevice();
		bool MeetsMinimimumRequirements(const VkPhysicalDevice& device);
		bool CheckDeviceExtensionSupport(const VkPhysicalDevice& device, const std::vector<const char*>& extensions);
		void IdentifyQueueFamilies(const VkPhysicalDevice& device, QueueFamilyIndices& indices);
		
		void CreateSwapchain();
		bool CheckSwapchainSupport(const VkPhysicalDevice& device, VulkanDeviceSwapchainSupport& support);
		VkSurfaceFormatKHR ChooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapchainPresentationMode(const std::vector<VkPresentModeKHR>& availableModes);
		VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities);


		#if Z_DEBUG
		bool m_ValidationLayersEnabled = true;
		#else
		bool m_ValidationLayersEnabled = false;
		#endif

	};
}
