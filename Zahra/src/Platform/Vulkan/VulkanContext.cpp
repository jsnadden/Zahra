#include "zpch.h"
#include "VulkanContext.h"

#include "Zahra/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Zahra
{
	namespace VulkanUtils
	{

		static void ValidateVkResult(VkResult result, const std::string& errorMessage = "")
		{
			if (result != VK_SUCCESS)
			{
				throw std::runtime_error(errorMessage);
			}
		}

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

	VulkanContext::VulkanContext(GLFWwindow* handle)
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

		ChoosePhysicalDevice();

	}

	void VulkanContext::Shutdown()
	{
		Z_CORE_INFO("Vulkan renderer shutting down");

		if (m_ValidationLayersEnabled)
		{
			VulkanUtils::DestroyDebugUtilsMessengerEXT(m_VulkanInstance, m_DebugMessenger, nullptr);
		}

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
		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

		if (m_ValidationLayersEnabled) CheckValidationLayerSupport(validationLayers);

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
			instanceCreateInfo.enabledLayerCount = (uint32_t)(validationLayers.size());
			instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

			PopulateDebugMessengerCreateInfo(instanceCreationDebugInfo);
			instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&instanceCreationDebugInfo;
		}
		else
		{
			instanceCreateInfo.enabledLayerCount = 0;

			instanceCreateInfo.pNext = nullptr;
		}

		CheckExtensionSupport(extensions);
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

		for (int i = 0; i < glfwExtensionCount; i++) extensions.emplace_back(glfwExtensions[i]);

	}

	void VulkanContext::CheckExtensionSupport(const std::vector<const char*>& extensions)
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
				std::string errorMessage = "Requested Vulkan extension \'";
				errorMessage += ext;
				errorMessage += "\' is not supported";
				Z_CORE_CRITICAL(errorMessage);
				throw std::runtime_error(errorMessage);
			}
		}

		Z_CORE_INFO("Requested Vulkan extensions are supported");

	}

	void VulkanContext::CheckValidationLayerSupport(const std::vector<const char*>& validationLayers)
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
				throw std::runtime_error(errorMessage);
			}
		}

		Z_CORE_INFO("Requested Vulkan validation layers are available");
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
		debugMessengerInfo.pfnUserCallback = DebugCallback;
	}

	void VulkanContext::ChoosePhysicalDevice()
	{
		uint32_t deviceCount = 0;
		VulkanUtils::ValidateVkResult(vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, nullptr));

		if (deviceCount == 0)
		{
			const char* errorMessage = "Failed to identify a GPU with Vulkan support";
			Z_CORE_CRITICAL(errorMessage);
			throw std::runtime_error(errorMessage);
		}

		std::vector<VkPhysicalDevice> allDevices(deviceCount);
		VulkanUtils::ValidateVkResult(vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, allDevices.data()));

		uint32_t highScore = 0;
		std::string deviceName = "";

		for (const auto& device : allDevices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);

			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			VkPhysicalDeviceMemoryProperties deviceMemory;
			vkGetPhysicalDeviceMemoryProperties(device, &deviceMemory);

			uint32_t score = ScorePhysicalDevice(deviceProperties, deviceFeatures, deviceMemory);

			if (score > highScore)
			{
				m_PhysicalDevice = device;
				deviceName = deviceProperties.deviceName;
				highScore = score;
			}
		}


		// Check if the best candidate is suitable at all
		if (m_PhysicalDevice == VK_NULL_HANDLE)
		{
			const char* errorMessage = "Failed to identify a GPU meeting the minimum required specifications";
			Z_CORE_CRITICAL(errorMessage);
			throw std::runtime_error(errorMessage);
		}

		Z_CORE_INFO("Vulkan will target the following GPU: {0}", deviceName.c_str());
	}

	uint32_t VulkanContext::ScorePhysicalDevice(VkPhysicalDeviceProperties properties, VkPhysicalDeviceFeatures features, VkPhysicalDeviceMemoryProperties memory)
	{
		// TODO: rewrite this!!!

		int score = 0;

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}

		score += properties.limits.maxImageDimension2D;
		score += memory.memoryHeapCount;

		return score;
	}



}

