#pragma once

#include "Platform/Vulkan/VulkanUtils.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Zahra/Core/Application.h"
#include "Zahra/Renderer/Renderer.h"
#include "Zahra/Renderer/RendererContext.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	class VulkanContext : public RendererContext
	{
	public:
		VulkanContext(GLFWwindow* handle);

		virtual void Init() override;
		virtual void Shutdown() override;
		
		VkInstance& GetVulkanInstance() { return m_VulkanInstance; }
		Ref<VulkanSwapchain> GetSwapchain() { return m_Swapchain; }
		Ref<VulkanDevice> GetDevice() { return m_Device; }
		VkPhysicalDevice& GetPhysicalDevice() { return m_Device->m_PhysicalDevice; }
		VkDevice& GetVkDevice() { return m_Device->m_LogicalDevice; }

		static Ref<VulkanContext> Get() { return Ref<VulkanContext>(Renderer::GetContext()); }
		static Ref<VulkanDevice> GetCurrentDevice() { return Get()->GetDevice(); }
		static VkDevice& GetCurrentVkDevice() { return Get()->GetVkDevice(); }

	private:
		VkInstance m_VulkanInstance = VK_NULL_HANDLE;

		GLFWwindow* m_WindowHandle;
		
		Ref<VulkanSwapchain> m_Swapchain;
		Ref<VulkanDevice> m_Device;

		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

		void CreateInstance();
		void GetGLFWExtensions(std::vector<const char*>& extensions);
		bool CheckInstanceExtensionSupport(const std::vector<const char*>& extensions);
		bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers);

		void CreateDebugMessenger();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		
		#if Z_DEBUG
		bool m_ValidationLayersEnabled = true;
		#else
		bool m_ValidationLayersEnabled = false;
		#endif

	};
}
