#include "zpch.h"
#include "VulkanContext.h"

#include "Zahra/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Zahra
{
	namespace VulkanUtils
	{
		static VkResult CreateDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
			const VkAllocationCallbacks* allocator,
			VkDebugUtilsMessengerEXT* debugMessenger)
		{
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			
			if (func != nullptr)
			{
				return func(instance, createInfo, allocator, debugMessenger);
			}
			else
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}

		static void DestroyDebugUtilsMessengerEXT(
			VkInstance instance,
			VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks* allocator)
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

			if (func != nullptr)
			{
				func(instance, debugMessenger, allocator);
			}
		}
	
	}

	static const std::vector<const char*> s_ValidationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	static VulkanContext* s_Instance = nullptr;

	VulkanContext* VulkanContext::Get(GLFWwindow* handle)
	{
		if (!s_Instance)
		{
			s_Instance = new VulkanContext(handle);
		}

		return s_Instance;
	}
	VulkanContext::VulkanContext(GLFWwindow* handle)
		: m_WindowHandle(handle)
	{
		Z_CORE_ASSERT(handle, "Window handle is NULL");
	}

	void VulkanContext::Init()
	{
		CreateInstance();

		if (m_ValidationLayersEnabled)
		{
			CreateDebugMessenger();
		}

		m_Swapchain = CreateRef<VulkanSwapchain>();
		m_Swapchain->Init(m_VulkanInstance, m_WindowHandle);
	}

	void VulkanContext::Shutdown()
	{
		Z_CORE_INFO("Vulkan renderer shutting down");

		m_Swapchain->Shutdown(m_VulkanInstance);

		if (m_ValidationLayersEnabled)
			VulkanUtils::DestroyDebugUtilsMessengerEXT(m_VulkanInstance, m_DebugMessenger, nullptr);

		// The instance must only be destroyed AFTER all the other Vulkan resources
		vkDestroyInstance(m_VulkanInstance, nullptr);
	}

	void VulkanContext::SwapBuffers()
	{

	}

	void VulkanContext::CreateInstance()
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////
		// SETUP VALIDATION LAYERS
		if (m_ValidationLayersEnabled)
		{
			bool supported = CheckValidationLayerSupport(s_ValidationLayers);
			if (!supported)
			{
				Z_CORE_INFO("Disabling validation layers");
				m_ValidationLayersEnabled = false;
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////
		// SPECIFY APPLICATION INFO
		ApplicationSpecification specification = Application::Get().GetSpecification();
		ApplicationVersion version = specification.Version;

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = specification.Name.c_str();
		appInfo.applicationVersion = VK_MAKE_API_VERSION(0, version.Major, version.Minor, version.Patch);
		appInfo.pEngineName = "Zahra";
		appInfo.engineVersion = 1; // TODO: might want to replace this (for now who cares)
		appInfo.apiVersion = VK_API_VERSION_1_3;

		//////////////////////////////////////////////////////////////////////////////////////////////////
		// CONFIGURE INSTANCE CREATION
		VkInstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &appInfo;
		
		std::vector<const char*> extensions;
		GetGLFWExtensions(extensions);

		VkDebugUtilsMessengerCreateInfoEXT instanceCreationDebugInfo{};

		if (m_ValidationLayersEnabled)
		{
			instanceCreateInfo.enabledLayerCount = (uint32_t)(s_ValidationLayers.size());
			instanceCreateInfo.ppEnabledLayerNames = s_ValidationLayers.data();
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

			PopulateDebugMessengerCreateInfo(instanceCreationDebugInfo);
			instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&instanceCreationDebugInfo;
		}
		else
		{
			instanceCreateInfo.enabledLayerCount = 0;

			instanceCreateInfo.pNext = nullptr;
		}

		if (!CheckInstanceExtensionSupport(extensions)) throw std::runtime_error("Vulkan instance creation failed");

		instanceCreateInfo.enabledExtensionCount = (uint32_t)(extensions.size());
		instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

		//////////////////////////////////////////////////////////////////////////////////////////////////
		// CREATE INSTANCE
		VulkanUtils::ValidateVkResult(vkCreateInstance(&instanceCreateInfo, nullptr, &m_VulkanInstance), "Vulkan instance creation failed");
		Z_CORE_INFO("Vulkan instance creation succeeded");
	}

	void VulkanContext::GetGLFWExtensions(std::vector<const char*>& extensions)
	{
		// Get all Vulkan extensions required by GLFW
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		for (uint32_t i = 0; i < glfwExtensionCount; i++) extensions.emplace_back(glfwExtensions[i]);

	}

	bool VulkanContext::CheckInstanceExtensionSupport(const std::vector<const char*>& extensions)
	{
		// Generate list of all supported extensions
		uint32_t supportedExtensionCount = 0;
		VulkanUtils::ValidateVkResult(vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr));
		
		std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
		VulkanUtils::ValidateVkResult(vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions.data()));

		for (auto& ext : extensions)
		{
			bool supported = false;

			for (auto& prop : supportedExtensions)
			{
				if (strncmp(ext, prop.extensionName, strlen(ext)) == 0)
				{
					supported = true;
					break;
				}
			}

			if (!supported)
			{
				std::string errorMessage = "Requested Vulkan instance extension \'";
				errorMessage += ext;
				errorMessage += "\' is not supported";
				Z_CORE_CRITICAL(errorMessage);
				return false;
			}
		}

		return true;
	}

	bool VulkanContext::CheckValidationLayerSupport(const std::vector<const char*>& validationLayers)
	{
		uint32_t layerCount;
		VulkanUtils::ValidateVkResult(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

		std::vector<VkLayerProperties> availableLayers(layerCount);
		VulkanUtils::ValidateVkResult(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));

		for (auto& layer : validationLayers)
		{
			bool available = false;

			for (auto& prop : availableLayers)
			{
				if (strncmp(layer, prop.layerName, strlen(layer)) == 0)
				{
					available = true;
					break;
				}
			}
			if (!available)
			{
				std::string errorMessage = "Vulkan validation layer \'";
				errorMessage += layer;
				errorMessage += "\' is unavailable";
				Z_CORE_CRITICAL(errorMessage);
				return false;
			}
		}

		return true;
	}

	void VulkanContext::CreateDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo{};
		PopulateDebugMessengerCreateInfo(debugMessengerInfo);
		debugMessengerInfo.pUserData = nullptr; // TODO: make use of optional user-defined data field

		VulkanUtils::ValidateVkResult(VulkanUtils::CreateDebugUtilsMessengerEXT(m_VulkanInstance, &debugMessengerInfo, nullptr, &m_DebugMessenger), "Vulkan debug messenger creation failed");
	}

	void VulkanContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugMessengerInfo)
	{
		debugMessengerInfo = {};
		debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugMessengerInfo.pfnUserCallback = VulkanUtils::DebugCallback;
	}

}

