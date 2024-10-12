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

		virtual void PresentImage() override;
		
		static Ref<VulkanContext> Get() { return Ref<VulkanContext>(Renderer::GetContext()); }
		Ref<VulkanSwapchain> GetSwapchain() { return m_Swapchain; }
		VkSurfaceKHR& GetSurface() { return m_Swapchain->GetSurface(); }
		Ref<VulkanDevice> GetDevice() { return m_Device; }
		static Ref<VulkanDevice> GetCurrentDevice() { return Get()->GetDevice(); }

	private:

		GLFWwindow* m_WindowHandle;

		VkInstance m_VulkanInstance = VK_NULL_HANDLE;
		
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
