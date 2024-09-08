#include "zpch.h"
#include "VulkanContext.h"

#include "Zahra/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Zahra
{
	namespace VulkanUtils
	{

		static void ValidateResult(VkResult result, const std::string& errorMessage)
		{
			if (result != VK_SUCCESS) {
				throw std::runtime_error(errorMessage);
			}
		}

	}

	VulkanContext::VulkanContext(GLFWwindow* handle)
	{
		Z_CORE_ASSERT(handle, "Window handle is NULL");
	}

	void VulkanContext::Init()
	{
		ApplicationSpecification specification = Application::Get().GetSpecification();
		ApplicationVersion version = specification.Version;

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = specification.Name.c_str();
		appInfo.applicationVersion = VK_MAKE_API_VERSION(0, version.Major, version.Minor, version.Patch);
		appInfo.pEngineName = "Zahra";
		appInfo.engineVersion = 1; // TODO: replace this (if having an engine version ever matters...)
		appInfo.apiVersion = VK_API_VERSION_1_3;

		// Get number and names of the Vulkan extensions required by GLFW
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		createInfo.enabledLayerCount = 0;

		VulkanUtils::ValidateResult(vkCreateInstance(&createInfo, nullptr, &m_VulkanInstance), "Vulkan instance creation failed");
		Z_CORE_INFO("Vulkan instance creation succeeded");
	}

	void VulkanContext::Shutdown()
	{


		// The instance must only be destroyed AFTER all the other Vulkan resources
		vkDestroyInstance(m_VulkanInstance, nullptr);
	}

	void VulkanContext::SwapBuffers()
	{

	}

}

