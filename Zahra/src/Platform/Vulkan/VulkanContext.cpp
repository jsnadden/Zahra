#include "zpch.h"
#include "VulkanContext.h"

#include "Zahra/Core/Application.h"

#include <GLFW/glfw3.h>
#include <set>

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

	static const std::vector<const char*> s_DeviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

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

		CreateSurface();
		CreateDevice();

		m_Swapchain = CreateRef<VulkanSwapchain>();
		m_Swapchain->Init(m_Device, m_Surface);
	}

	void VulkanContext::Shutdown()
	{
		Z_CORE_INFO("Vulkan renderer shutting down");

		m_Swapchain->Shutdown();

		ShutdownDevice();
		vkDestroySurfaceKHR(m_VulkanInstance, m_Surface, nullptr);

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

	void VulkanContext::CreateSurface()
	{
		VulkanUtils::ValidateVkResult(glfwCreateWindowSurface(m_VulkanInstance, m_WindowHandle, nullptr, &m_Surface), "Vulkan surface creation failed");
		Z_CORE_INFO("Vulkan surface creation succeeded");
	}

	void VulkanContext::CreateDevice()
	{
		m_Device = CreateRef<VulkanDevice>();

		TargetPhysicalDevice();

		std::vector<VkDeviceQueueCreateInfo> queueInfoList;

		// using a set rather than a vector here, because assigning separate
		// queues to the same index will cause device creation to fail
		std::set<uint32_t> queueIndices =
		{
			m_Device->QueueFamilyIndices.GraphicsIndex.value(),
			m_Device->QueueFamilyIndices.PresentationIndex.value()
		};		

		float queuePriority = 1.0f;

		for (uint32_t index : queueIndices)
		{
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = index;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &queuePriority;

			queueInfoList.push_back(queueInfo);
		}

		VkDeviceCreateInfo logicalDeviceInfo{};
		logicalDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		logicalDeviceInfo.queueCreateInfoCount = (uint32_t)queueInfoList.size();
		logicalDeviceInfo.pQueueCreateInfos = queueInfoList.data();
		logicalDeviceInfo.pEnabledFeatures = &m_Device->Features;
		logicalDeviceInfo.enabledExtensionCount = (uint32_t)s_DeviceExtensions.size();
		logicalDeviceInfo.ppEnabledExtensionNames = s_DeviceExtensions.data();

		VulkanUtils::ValidateVkResult(vkCreateDevice(m_Device->PhysicalDevice, &logicalDeviceInfo, nullptr, &m_Device->Device), "Vulkan device creation failed");
		Z_CORE_INFO("Vulkan device creation succeeded");
		Z_CORE_INFO("Target GPU: {0}", m_Device->Properties.deviceName);

		vkGetDeviceQueue(m_Device->Device, m_Device->QueueFamilyIndices.GraphicsIndex.value(), 0, &m_Device->GraphicsQueue);
		vkGetDeviceQueue(m_Device->Device, m_Device->QueueFamilyIndices.PresentationIndex.value(), 0, &m_Device->PresentationQueue);

	}

	void VulkanContext::ShutdownDevice()
	{
		vkDestroyDevice(m_Device->Device, nullptr);

	}

	void VulkanContext::TargetPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		VulkanUtils::ValidateVkResult(vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, nullptr));

		if (deviceCount == 0)
		{
			const char* errorMessage = "Failed to identify a GPU with Vulkan support";
			Z_CORE_CRITICAL(errorMessage);
			throw std::runtime_error(errorMessage);
		}

		QueueFamilyIndices indices;

		std::vector<VkPhysicalDevice> allDevices(deviceCount);
		VulkanUtils::ValidateVkResult(vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, allDevices.data()));

		for (const auto& device : allDevices)
		{
			bool pass = MeetsMinimimumRequirements(device);

			VulkanDeviceSwapchainSupport support;
			pass &= CheckSwapchainSupport(device, support);

			if (!pass) break;

			IdentifyQueueFamilies(device, indices);

			if (indices.Complete())
			{
				m_Device->PhysicalDevice = device;
				m_Device->QueueFamilyIndices = indices;
				m_Device->SwapchainSupport = support;

				vkGetPhysicalDeviceFeatures(device, &m_Device->Features);
				vkGetPhysicalDeviceProperties(device, &m_Device->Properties);
				vkGetPhysicalDeviceMemoryProperties(device, &m_Device->Memory);
			}

		}

		if (m_Device->PhysicalDevice == VK_NULL_HANDLE)
		{
			const char* errorMessage = "Failed to identify a GPU meeting the minimum required specifications";
			Z_CORE_CRITICAL(errorMessage);
			throw std::runtime_error(errorMessage);
		}

	}

	bool VulkanContext::MeetsMinimimumRequirements(const VkPhysicalDevice& device)
	{
		if (!CheckDeviceExtensionSupport(device, s_DeviceExtensions)) return false;

		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(device, &features);
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);
		VkPhysicalDeviceMemoryProperties memory;
		vkGetPhysicalDeviceMemoryProperties(device, &memory);

		const GPURequirements& requirements = Application::Get().GetSpecification().MinGPURequirements;

		bool pass = true;

		if (requirements.IsDiscreteGPU)
			pass &= properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

		// TODO: add checks for other requirements

		return pass;
	}

	bool VulkanContext::CheckDeviceExtensionSupport(const VkPhysicalDevice& device, const std::vector<const char*>& extensions)
	{
		// Generate list of all supported extensions
		uint32_t supportedExtensionCount = 0;
		VulkanUtils::ValidateVkResult(vkEnumerateDeviceExtensionProperties(device, nullptr, &supportedExtensionCount, nullptr));

		std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
		VulkanUtils::ValidateVkResult(vkEnumerateDeviceExtensionProperties(device, nullptr, &supportedExtensionCount, supportedExtensions.data()));

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

			if (!supported) return false;

		}

		return true;
	}

	void VulkanContext::IdentifyQueueFamilies(const VkPhysicalDevice& device, QueueFamilyIndices& indices)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		// TODO: take other queues into account, and optimise these choices

		for (uint32_t i = 0; i < queueFamilyCount; i++)
		{
			if (indices.Complete()) break;

			VkBool32 presentationSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentationSupport);

			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.GraphicsIndex = i;
			if (presentationSupport) indices.PresentationIndex = i;
		}
	}

	bool VulkanContext::CheckSwapchainSupport(const VkPhysicalDevice& device, VulkanDeviceSwapchainSupport& support)
	{
		bool adequateSupport = true;

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);
		
		if (formatCount)
		{
			support.Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, support.Formats.data());
		}
		else
		{
			adequateSupport = false;
		}

		uint32_t modeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &modeCount, nullptr);

		if (modeCount)
		{
			support.PresentationModes.resize(modeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &modeCount, support.PresentationModes.data());
		}
		else
		{
			adequateSupport = false;
		}

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &support.Capabilities);

		if (!adequateSupport) Z_CORE_CRITICAL("Selected GPU/window do not provide adequate support for Vulkan swap chain creation");
		return adequateSupport;

	}

}

