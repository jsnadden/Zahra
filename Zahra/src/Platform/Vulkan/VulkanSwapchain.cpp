#include "zpch.h"
#include "VulkanSwapchain.h"

#include "Platform/Vulkan/VulkanUtils.h"
#include "Zahra/Core/Application.h"
#include "Zahra/Renderer/Renderer.h"

#include <GLFW/glfw3.h>
#include <set>

namespace Zahra
{
	

	static const std::vector<const char*> s_DeviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	void VulkanSwapchain::Init(VkInstance& instance, GLFWwindow* windowHandle)
	{
		CreateSurface(instance, windowHandle);
		CreateDevice(instance);
		CreateSwapchain();
		GetSwapchainImagesAndCreateImageViews();
		CreateCommandPool();
		AllocateCommandBuffer();
		CreateSyncObjects();

		Z_CORE_TRACE("Vulkan swap chain creation succeeded");
	}

	void VulkanSwapchain::Invalidate()
	{
		vkDeviceWaitIdle(m_Device->m_LogicalDevice);

		Cleanup();

		CreateSwapchain();
		GetSwapchainImagesAndCreateImageViews();

		m_Invalidated = true;
	}

	void VulkanSwapchain::Shutdown(VkInstance& instance)
	{
		Cleanup();

		for (uint32_t i = 0; i < m_FramesInFlight; i++)
		{
			vkDestroySemaphore(m_Device->m_LogicalDevice, m_ImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(m_Device->m_LogicalDevice, m_RenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(m_Device->m_LogicalDevice, m_InFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(m_Device->m_LogicalDevice, m_CommandPool, nullptr);

		m_Device->Shutdown();

		vkDestroySurfaceKHR(instance, m_Surface, nullptr);
		m_Surface = VK_NULL_HANDLE;
	}

	void VulkanSwapchain::Cleanup()
	{
		for (auto imageView : m_ImageViews)
		{
			vkDestroyImageView(m_Device->m_LogicalDevice, imageView, nullptr);
		}
		m_ImageViews.clear();

		vkDestroySwapchainKHR(m_Device->m_LogicalDevice, m_Swapchain, nullptr);
		m_Images.clear();
		m_Swapchain = VK_NULL_HANDLE;
	}

	void VulkanSwapchain::SignalResize()
	{
		m_WindowResized = true;
	}

	void VulkanSwapchain::GetNextImage()
	{
		vkWaitForFences(m_Device->m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrameIndex], VK_TRUE, UINT64_MAX);

		VkResult result = vkAcquireNextImageKHR(m_Device->m_LogicalDevice, m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrameIndex], VK_NULL_HANDLE, &m_CurrentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			Invalidate();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Vulkan swapchain failed to acquire next image");
		}

		vkResetFences(m_Device->m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrameIndex]);
		vkResetCommandBuffer(m_DrawCommandBuffers[m_CurrentFrameIndex], 0);
	}

	void VulkanSwapchain::ExecuteDrawCommandBuffer()
	{

	}

	void VulkanSwapchain::PresentImage()
	{

		/////////////////////////////////////////////////////////////////////////////////////////
		// SUBMIT DRAW COMMAND BUFFER FOR EXECUTION
		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrameIndex] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrameIndex] };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_DrawCommandBuffers[m_CurrentFrameIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		VulkanUtils::ValidateVkResult(vkQueueSubmit(m_Device->m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrameIndex]));

		/////////////////////////////////////////////////////////////////////////////////////////
		// WAIT FOR RENDERING TO COMPLETE, THEN DISPLAY SWAPCHAIN IMAGE

		VkSwapchainKHR swapChains[] = { m_Swapchain };

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &m_CurrentImageIndex;

		VkResult result = vkQueuePresentKHR(m_Device->m_PresentationQueue, &presentInfo);

		m_Invalidated = false;

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_WindowResized)
		{
			m_WindowResized = false;
			Invalidate();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Vulkan swapchain failed to present rendered image");
		}

		m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_FramesInFlight;

	}

	VkCommandBuffer VulkanSwapchain::GetDrawCommandBuffer(uint32_t index)
	{
		Z_CORE_ASSERT(index < m_DrawCommandBuffers.size());

		return m_DrawCommandBuffers[index];
	}

	void VulkanSwapchain::CreateSurface(VkInstance& instance, GLFWwindow* windowHandle)
	{
		VulkanUtils::ValidateVkResult(glfwCreateWindowSurface(instance, windowHandle, nullptr, &m_Surface), "Vulkan surface creation failed");
		Z_CORE_TRACE("Vulkan surface creation succeeded");
	}

	void VulkanSwapchain::CreateDevice(VkInstance& instance)
	{
		m_Device = Ref<VulkanDevice>::Create();

		TargetPhysicalDevice(instance);

		std::vector<VkDeviceQueueCreateInfo> queueInfoList;

		// using a set rather than a vector here, because assigning separate
		// queues to the same index will cause device creation to fail
		std::set<uint32_t> queueIndices =
		{
			m_Device->m_QueueFamilyIndices.GraphicsIndex.value(),
			m_Device->m_QueueFamilyIndices.PresentIndex.value()
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

		VkPhysicalDeviceFeatures enabledFeatures{};
		enabledFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo logicalDeviceInfo{};
		logicalDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		logicalDeviceInfo.queueCreateInfoCount = (uint32_t)queueInfoList.size();
		logicalDeviceInfo.pQueueCreateInfos = queueInfoList.data();
		logicalDeviceInfo.pEnabledFeatures = &enabledFeatures;
		logicalDeviceInfo.enabledExtensionCount = (uint32_t)s_DeviceExtensions.size();
		logicalDeviceInfo.ppEnabledExtensionNames = s_DeviceExtensions.data();

		VulkanUtils::ValidateVkResult(vkCreateDevice(m_Device->m_PhysicalDevice, &logicalDeviceInfo, nullptr, &m_Device->m_LogicalDevice), "Vulkan device creation failed");
		Z_CORE_TRACE("Vulkan device creation succeeded");
		Z_CORE_INFO("Target GPU: {0}", m_Device->m_Properties.deviceName);

		vkGetDeviceQueue(m_Device->m_LogicalDevice, m_Device->m_QueueFamilyIndices.GraphicsIndex.value(), 0, &m_Device->m_GraphicsQueue);
		vkGetDeviceQueue(m_Device->m_LogicalDevice, m_Device->m_QueueFamilyIndices.PresentIndex.value(), 0, &m_Device->m_PresentationQueue);
	}

	void VulkanSwapchain::TargetPhysicalDevice(VkInstance& instance)
	{
		uint32_t deviceCount = 0;
		VulkanUtils::ValidateVkResult(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));

		if (deviceCount == 0)
		{
			const char* errorMessage = "Failed to identify a GPU with Vulkan support";
			Z_CORE_CRITICAL(errorMessage);
			throw std::runtime_error(errorMessage);
		}

		QueueFamilyIndices indices;

		std::vector<VkPhysicalDevice> allDevices(deviceCount);
		VulkanUtils::ValidateVkResult(vkEnumeratePhysicalDevices(instance, &deviceCount, allDevices.data()));

		for (const auto& device : allDevices)
		{
			bool pass = MeetsMinimimumRequirements(device);

			VulkanDeviceSwapchainSupport support;
			pass &= CheckSwapchainSupport(device, support);

			if (!pass) break;

			IdentifyQueueFamilies(device, indices);

			if (indices.Complete())
			{
				m_Device->m_PhysicalDevice = device;
				m_Device->m_QueueFamilyIndices = indices;
				m_Device->m_SwapchainSupport = support;

				vkGetPhysicalDeviceFeatures(device, &m_Device->m_Features);
				vkGetPhysicalDeviceProperties(device, &m_Device->m_Properties);
				vkGetPhysicalDeviceMemoryProperties(device, &m_Device->m_MemoryProperties);
			}

		}

		if (m_Device->m_PhysicalDevice == VK_NULL_HANDLE)
		{
			const char* errorMessage = "Failed to identify a GPU meeting the minimum required specifications";
			Z_CORE_CRITICAL(errorMessage);
			throw std::runtime_error(errorMessage);
		}
	}

	bool VulkanSwapchain::MeetsMinimimumRequirements(const VkPhysicalDevice& device)
	{
		if (!CheckDeviceExtensionSupport(device, s_DeviceExtensions)) return false;

		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(device, &features);
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);
		VkPhysicalDeviceMemoryProperties memory;
		vkGetPhysicalDeviceMemoryProperties(device, &memory);

		const GPURequirements& requirements = Application::Get().GetSpecification().GPURequirements;

		if (requirements.IsDiscreteGPU && properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			return false;

		if (requirements.AnisotropicFiltering && features.samplerAnisotropy == VK_FALSE)
			return false;

		if (properties.limits.maxDescriptorSetSampledImages < requirements.MinBoundTextureSlots)
			return false;

		// TODO: add checks for other requirements

		return true;
	}

	bool VulkanSwapchain::CheckDeviceExtensionSupport(const VkPhysicalDevice& device, const std::vector<const char*>& extensions)
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

	void VulkanSwapchain::IdentifyQueueFamilies(const VkPhysicalDevice& device, QueueFamilyIndices& indices)
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
			if (presentationSupport) indices.PresentIndex = i;
		}
	}

	bool VulkanSwapchain::CheckSwapchainSupport(const VkPhysicalDevice& device, VulkanDeviceSwapchainSupport& support)
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

		QuerySurfaceCapabilities(device, support.Capabilities);

		if (!adequateSupport) Z_CORE_CRITICAL("Selected GPU/window do not provide adequate support for Vulkan swap chain creation");
		return adequateSupport;
	}

	void VulkanSwapchain::QuerySurfaceCapabilities(const VkPhysicalDevice& device, VkSurfaceCapabilitiesKHR& capabilities)
	{
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &capabilities);
	}

	void VulkanSwapchain::CreateSwapchain()
	{
		QuerySurfaceCapabilities(m_Device->m_PhysicalDevice, m_Device->m_SwapchainSupport.Capabilities);

		m_SurfaceFormat = ChooseSwapchainFormat();
		m_PresentationMode = ChooseSwapchainPresentationMode();
		m_Extent = ChooseSwapchainExtent();

		m_ImageCount = m_Device->m_SwapchainSupport.Capabilities.minImageCount + 1;
		uint32_t maxImageCount = m_Device->m_SwapchainSupport.Capabilities.maxImageCount;
		if (maxImageCount > 0 && m_ImageCount > maxImageCount) m_ImageCount = maxImageCount;
		
		VkSwapchainCreateInfoKHR swapchainInfo{};
		swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainInfo.surface = m_Surface;
		swapchainInfo.minImageCount = m_ImageCount;
		swapchainInfo.imageFormat = m_SurfaceFormat.format;
		swapchainInfo.imageColorSpace = m_SurfaceFormat.colorSpace;
		swapchainInfo.presentMode = m_PresentationMode;
		swapchainInfo.imageExtent = m_Extent;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] =
		{
			m_Device->m_QueueFamilyIndices.GraphicsIndex.value(),
			m_Device->m_QueueFamilyIndices.PresentIndex.value()
		};

		if (m_Device->m_QueueFamilyIndices.GraphicsIndex = m_Device->m_QueueFamilyIndices.PresentIndex)
		{
			// EXCLUSIVE MODE: An image is owned by one queue family
			// at a time and ownership must be explicitly transferred
			// before using it in another queue family. This option
			// offers the best performance.
			swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainInfo.queueFamilyIndexCount = 0;
			swapchainInfo.pQueueFamilyIndices = nullptr;
		}
		else
		{
			// CONCURRENT MODE: Images can be used across multiple queue
			// families without explicit ownership transfers. This requires
			// you to specify in advance between which queue families
			// ownership will be shared using the queueFamilyIndexCount
			// and pQueueFamilyIndices parameters. Could also use exclusive
			// mode in this case, but it's more complicated.
			swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainInfo.queueFamilyIndexCount = 2;
			swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		swapchainInfo.preTransform = m_Device->m_SwapchainSupport.Capabilities.currentTransform; // no overall screen rotation/flip
		swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // no alpha blending with other windows
		swapchainInfo.clipped = VK_TRUE; // ignore pixels obscured by other windows

		// With Vulkan it's possible that your swap chain becomes invalid
		// or unoptimized while your application is running, for example
		// because the window was resized. In that case the swap chain
		// actually needs to be recreated from scratch and a reference to
		// the old one must be specified in this field.
		swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

		VulkanUtils::ValidateVkResult(vkCreateSwapchainKHR(m_Device->m_LogicalDevice, &swapchainInfo, nullptr, &m_Swapchain),
			"Vulkan swap chain creation failed");
	}

	VkSurfaceFormatKHR VulkanSwapchain::ChooseSwapchainFormat()
	{
		for (const auto& format : m_Device->m_SwapchainSupport.Formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}

		// TODO: rank formats and return best. For now just...
		return m_Device->m_SwapchainSupport.Formats[0];
	}

	VkPresentModeKHR VulkanSwapchain::ChooseSwapchainPresentationMode()
	{
		for (const auto& mode : m_Device->m_SwapchainSupport.PresentationModes)
		{
			// triple+ buffering
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return mode;
			}
		}

		// TODO: more detailed preferencing

		// standard double buffering
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanSwapchain::ChooseSwapchainExtent()
	{
		const auto& capabilities = m_Device->m_SwapchainSupport.Capabilities;

		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}

		GLFWwindow* windowHandle = Application::Get().GetWindow().GetWindowHandle();

		int width, height;
		glfwGetFramebufferSize(windowHandle, &width, &height);

		VkExtent2D extent = { (uint32_t)width, (uint32_t)height };

		extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return extent;
	}

	void VulkanSwapchain::GetSwapchainImagesAndCreateImageViews()
	{
		vkGetSwapchainImagesKHR(m_Device->m_LogicalDevice, m_Swapchain, &m_ImageCount, nullptr);
		m_Images.resize(m_ImageCount);
		vkGetSwapchainImagesKHR(m_Device->m_LogicalDevice, m_Swapchain, &m_ImageCount, m_Images.data());

		m_FramesInFlight = Renderer::GetConfig().FramesInFlight;
		if (m_FramesInFlight > m_ImageCount) m_FramesInFlight = m_ImageCount;

		m_ImageViews.resize(m_ImageCount);
		for (size_t i = 0; i < m_ImageCount; i++)
			m_ImageViews[i] = m_Device->CreateVulkanImageView(m_SurfaceFormat.format, m_Images[i], VK_IMAGE_ASPECT_COLOR_BIT);
	}

	void VulkanSwapchain::CreateCommandPool()
	{
		VkCommandPoolCreateInfo graphicsPoolInfo{};
		graphicsPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		graphicsPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		graphicsPoolInfo.queueFamilyIndex = m_Device->m_QueueFamilyIndices.GraphicsIndex.value();

		VulkanUtils::ValidateVkResult(vkCreateCommandPool(m_Device->m_LogicalDevice, &graphicsPoolInfo, nullptr, &m_CommandPool),
			"Vulkan command pool creation failed");
	}

	void VulkanSwapchain::AllocateCommandBuffer()
	{
		m_DrawCommandBuffers.resize(m_FramesInFlight);

		VkCommandBufferAllocateInfo commandBufferInfo{};
		commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferInfo.commandPool = m_CommandPool;
		commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferInfo.commandBufferCount = m_FramesInFlight;

		VulkanUtils::ValidateVkResult(vkAllocateCommandBuffers(m_Device->m_LogicalDevice, &commandBufferInfo, m_DrawCommandBuffers.data()),
			"Vulkan command buffer allocations failed");
	}

	void VulkanSwapchain::CreateSyncObjects()
	{
		VkDevice device = m_Device->m_LogicalDevice;

		m_ImageAvailableSemaphores.resize(m_FramesInFlight);
		m_RenderFinishedSemaphores.resize(m_FramesInFlight);
		m_InFlightFences.resize(m_FramesInFlight);

		// these guys have trivial CreateInfos
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		// fence starts in the signaled state, otherwise we would wait indefinitely on the first frame:
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (uint32_t i = 0; i < m_FramesInFlight; i++)
		{
			VulkanUtils::ValidateVkResult(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]),
				"Vulkan semaphore creation failed");
			VulkanUtils::ValidateVkResult(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]),
				"Vulkan semaphore creation failed");
			VulkanUtils::ValidateVkResult(vkCreateFence(device, &fenceInfo, nullptr, &m_InFlightFences[i]),
				"Vulkan fence creation failed");
		}		

	}

}

