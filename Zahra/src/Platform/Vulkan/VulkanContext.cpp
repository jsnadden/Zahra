#include "zpch.h"
#include "VulkanContext.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	
	VulkanContext::VulkanContext(GLFWwindow* handle)
		: m_WindowHandle(handle)
	{
		Z_CORE_ASSERT(handle, "Window handle is NULL")
	}

	void VulkanContext::Init()
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		Z_CORE_INFO("{} Vulkan extensions supported", extensionCount);
	}

	void VulkanContext::SwapBuffers()
	{

	}

}

