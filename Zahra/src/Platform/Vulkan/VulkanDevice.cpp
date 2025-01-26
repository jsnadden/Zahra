#include "zpch.h"
#include "VulkanDevice.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUtils.h"
#include "Zahra/Core/Application.h"
#include "Zahra/Renderer/Renderer.h"

#include <GLFW/glfw3.h>

#include <set>

namespace Zahra
{

	namespace VulkanUtils
	{
		static bool FormatHasStencilComponent(VkFormat format)
		{
			return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
		}

		static uint32_t VkSampleCountFlagBitToSampleCount(VkSampleCountFlagBits bit)
		{
			switch (bit)
			{
				case VK_SAMPLE_COUNT_1_BIT:
				{
					return 1;
					break;
				}
				case VK_SAMPLE_COUNT_2_BIT:
				{
					return 2;
					break;
				}
				case VK_SAMPLE_COUNT_4_BIT:
				{
					return 4;
					break;
				}
				case VK_SAMPLE_COUNT_8_BIT:
				{
					return 8;
					break;
				}
				case VK_SAMPLE_COUNT_16_BIT:
				{
					return 16;
					break;
				}
				case VK_SAMPLE_COUNT_32_BIT:
				{
					return 32;
					break;
				}
				case VK_SAMPLE_COUNT_64_BIT:
				{
					return 64;
					break;
				}
			}
			Z_CORE_ASSERT(false, "Invalid VkSampleCountFlagBits");
			return 0;
		}

		static VkSampleCountFlagBits SampleCountToVkSampleCountFlagBit(uint32_t count)
		{
			switch (count)
			{
				case 1:
				{
					return VK_SAMPLE_COUNT_1_BIT;
					break;
				}
				case 2:
				{
					return VK_SAMPLE_COUNT_2_BIT;
					break;
				}
				case 4:
				{
					return VK_SAMPLE_COUNT_4_BIT;
					break;
				}
				case 8:
				{
					return VK_SAMPLE_COUNT_8_BIT;
					break;
				}
				case 16:
				{
					return VK_SAMPLE_COUNT_16_BIT;
					break;
				}
				case 32:
				{
					return VK_SAMPLE_COUNT_32_BIT;
					break;
				}
				case 64:
				{
					return VK_SAMPLE_COUNT_64_BIT;
					break;
				}
			}
			Z_CORE_ASSERT(false, "Sample count must be a power of 2, between 1 and 64");
			return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
		}
	}

	static const std::vector<const char*> s_DeviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VulkanDevice::VulkanDevice(VkInstance& instance, GLFWwindow* windowHandle)
	{
		Z_CORE_ASSERT(instance);

		VulkanUtils::ValidateVkResult(glfwCreateWindowSurface(instance, windowHandle, nullptr, &m_Surface), "Vulkan surface creation failed");
		//Z_CORE_TRACE("Vulkan surface creation succeeded");

		TargetPhysicalDevice(instance);

		std::vector<VkDeviceQueueCreateInfo> queueInfoList;

		// using a set rather than a vector here, because assigning separate
		// queues to the same index will cause device creation to fail
		std::set<uint32_t> queueIndices =
		{
			m_QueueFamilyIndices.GraphicsIndex.value(),
			m_QueueFamilyIndices.PresentIndex.value()
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

		auto desiredFeatures = DesiredFeatures();

		VkDeviceCreateInfo logicalDeviceInfo{};
		logicalDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		logicalDeviceInfo.queueCreateInfoCount = (uint32_t)queueInfoList.size();
		logicalDeviceInfo.pQueueCreateInfos = queueInfoList.data();
		logicalDeviceInfo.pEnabledFeatures = &desiredFeatures;
		logicalDeviceInfo.enabledExtensionCount = (uint32_t)s_DeviceExtensions.size();
		logicalDeviceInfo.ppEnabledExtensionNames = s_DeviceExtensions.data();

		VulkanUtils::ValidateVkResult(vkCreateDevice(m_PhysicalDevice, &logicalDeviceInfo, nullptr, &m_LogicalDevice), "Vulkan device creation failed");
		//Z_CORE_TRACE("Vulkan device creation succeeded");
		Z_CORE_INFO("Target GPU: {0}", m_Properties.deviceName);

		vkGetDeviceQueue(m_LogicalDevice, m_QueueFamilyIndices.GraphicsIndex.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, m_QueueFamilyIndices.PresentIndex.value(), 0, &m_PresentationQueue);
	}

	void VulkanDevice::Shutdown(VkInstance& instance)
	{
		for (auto& [threadID, pool] : m_CommandPools)
			vkDestroyCommandPool(m_LogicalDevice, pool, nullptr);
		m_CommandPools.clear();

		if (m_LogicalDevice) vkDestroyDevice(m_LogicalDevice, nullptr);
		m_LogicalDevice = VK_NULL_HANDLE;

		vkDestroySurfaceKHR(instance, m_Surface, nullptr);
	}

	void VulkanDevice::CreateVulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VulkanUtils::ValidateVkResult(vkCreateBuffer(m_LogicalDevice, &bufferInfo, nullptr, &buffer),
			"Vulkan buffer creation failed");

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(m_LogicalDevice, buffer, &memoryRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memoryRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, properties);

		VulkanUtils::ValidateVkResult(vkAllocateMemory(m_LogicalDevice, &allocInfo, nullptr, &bufferMemory),
			"Vulkan failed to allocate buffer memory");

		vkBindBufferMemory(m_LogicalDevice, buffer, bufferMemory, 0);
	}

	uint32_t VulkanDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags flags)
	{
		for (uint32_t i = 0; i < m_MemoryProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && ((m_MemoryProperties.memoryTypes[i].propertyFlags & flags) == flags))
				return i;
		}

		throw std::runtime_error("Vulkan device couldn't identify a suitable memory type");
	}

	void VulkanDevice::CopyVulkanBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset)
	{
		VkCommandBuffer commandBuffer = GetTemporaryCommandBuffer();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = srcOffset;
		copyRegion.dstOffset = dstOffset;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		// TODO: dedicated transfer queue?
		SubmitTemporaryCommandBuffer(commandBuffer);
	}

	void VulkanDevice::CreateVulkanImage(uint32_t width, uint32_t height, uint32_t mips, uint32_t samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		bool mutableFormat = Application::Get().GetSpecification().ImGuiConfig.ColourCorrectSceneTextures;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.flags = mutableFormat ? VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT : 0;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mips;
		imageInfo.arrayLayers = 1; // TODO: can be used for e.g. shadow map cascades
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VulkanUtils::SampleCountToVkSampleCountFlagBit(samples);
		
		VulkanUtils::ValidateVkResult(vkCreateImage(m_LogicalDevice, &imageInfo, nullptr, &image),
			"Vulkan image creation failed");

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(m_LogicalDevice, image, &memoryRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memoryRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VulkanUtils::ValidateVkResult(vkAllocateMemory(m_LogicalDevice, &allocInfo, nullptr, &imageMemory),
			"Vulkan failed to allocate image memory");

		vkBindImageMemory(m_LogicalDevice, image, imageMemory, 0);

	}

	VkImageView VulkanDevice::CreateVulkanImageView(VkFormat format, VkImage& image, VkImageAspectFlags aspectFlags, uint32_t mips)
	{
		VkImageView imageView;

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mips;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VulkanUtils::ValidateVkResult(vkCreateImageView(m_LogicalDevice, &viewInfo, nullptr, &imageView),
			"Vulkan image view creation failed");

		return imageView;
	}

	void VulkanDevice::CopyVulkanBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = GetTemporaryCommandBuffer();

		VkBufferImageCopy copyInfo{};
		copyInfo.bufferOffset = 0;
		copyInfo.bufferRowLength = 0;
		copyInfo.bufferImageHeight = 0;
		copyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyInfo.imageSubresource.mipLevel = 0;
		copyInfo.imageSubresource.baseArrayLayer = 0;
		copyInfo.imageSubresource.layerCount = 1;
		copyInfo.imageOffset = { 0, 0, 0 };
		copyInfo.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer, buffer, image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo); // this assumes we've already transitioned the image layout to TRANSFER_DST_OPTIMAL;

		SubmitTemporaryCommandBuffer(commandBuffer);

	}

	void VulkanDevice::CopyPixelToBuffer(VkImage image, VkBuffer buffer, int32_t x, int32_t y, int32_t mipLevel)
	{
		VkCommandBuffer commandBuffer = GetTemporaryCommandBuffer();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = mipLevel;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkBufferImageCopy copyInfo{};
		copyInfo.bufferOffset = 0;
		copyInfo.bufferRowLength = 0;
		copyInfo.bufferImageHeight = 0;
		copyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyInfo.imageSubresource.mipLevel = mipLevel;
		copyInfo.imageSubresource.baseArrayLayer = 0;
		copyInfo.imageSubresource.layerCount = 1;
		copyInfo.imageOffset = { x, y, 0 };
		copyInfo.imageExtent = { 1, 1, 1 };

		vkCmdCopyImageToBuffer(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &copyInfo); // this assumes we've already transitioned the image layout to TRANSFER_DST_OPTIMAL;

		SubmitTemporaryCommandBuffer(commandBuffer);
	}

	void VulkanDevice::CopyVulkanImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height)
	{
		// assumes:
		//  - appropriate layout transitions have been performed
		//  - images have the same dimensions
		//  - images have a colour format (not depth/stencil e.g.)

		VkCommandBuffer commandBuffer = GetTemporaryCommandBuffer();

		VkImageCopy copyRegion{};
		copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.srcSubresource.mipLevel = 0;
		copyRegion.srcSubresource.baseArrayLayer = 0;
		copyRegion.srcSubresource.layerCount = 1;
		copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.dstSubresource.mipLevel = 0;
		copyRegion.dstSubresource.baseArrayLayer = 0;
		copyRegion.dstSubresource.layerCount = 1;
		copyRegion.extent.width = width;
		copyRegion.extent.height = height;
		copyRegion.extent.depth = 1;

		vkCmdCopyImage(commandBuffer,
			srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &copyRegion);

		SubmitTemporaryCommandBuffer(commandBuffer);
	}

	void VulkanDevice::TransitionVulkanImageLayout(VkImage image, VkFormat format, uint32_t mips, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		if (oldLayout == newLayout)
			return;

		VkCommandBuffer commandBuffer = GetTemporaryCommandBuffer();

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;

		VkImageAspectFlags aspectFlags;
		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			aspectFlags = VulkanUtils::FormatHasStencilComponent(format) ?
				VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT :
				VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		else
		{
			aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // not transferring ownership
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = aspectFlags;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mips;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			// TODO: include other possibilities
			Z_CORE_ASSERT(false, "The requested layout transition is not currently supported");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			srcStage, dstStage,		// pipeline stage flags
			0,						// dependency flags
			0, nullptr,				// memory barriers
			0, nullptr,				// buffer memory barriers
			1, &barrier				// image memory barriers
		);

		SubmitTemporaryCommandBuffer(commandBuffer);
	}

	VkCommandBuffer VulkanDevice::GetTemporaryCommandBuffer(bool begin)
	{
		VkCommandPool commandPool = GetOrCreateCommandPool();

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, &commandBuffer);

		if (begin)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);
		}

		return commandBuffer;
	}

	void VulkanDevice::SubmitTemporaryCommandBuffer(VkCommandBuffer commandBuffer, GPUQueueType queueType)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VkQueue queue = VK_NULL_HANDLE;

		switch (queueType)
		{
			case GPUQueueType::Graphics:
			{
				queue = m_GraphicsQueue;
				break;
			}
			case GPUQueueType::Present:
			{
				queue = m_PresentationQueue;
				break;
			}
			case GPUQueueType::Transfer:
			{
				queue = m_TransferQueue;
				break;
			}
			case GPUQueueType::Compute:
			{
				queue = m_ComputeQueue;
				break;
			}

			default:
				Z_CORE_ASSERT(false, "Unrecognised queue type");
		}

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);
		// TODO: synchronise using fences instead of just waiting ("A fence would allow
		// you to schedule multiple transfers simultaneously and wait for all of them
		// complete, instead of executing one at a time. That may give the driver more
		// opportunities to optimize")

		VkCommandPool commandPool = GetOrCreateCommandPool();
		vkFreeCommandBuffers(m_LogicalDevice, commandPool, 1, &commandBuffer);
	}

	VkCommandBuffer VulkanDevice::GetSecondaryCommandBuffer()
	{
		VkCommandBuffer cmdBuffer;

		VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
		commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocInfo.commandPool = GetOrCreateCommandPool();
		commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		commandBufferAllocInfo.commandBufferCount = 1;

		VulkanUtils::ValidateVkResult(vkAllocateCommandBuffers(m_LogicalDevice, &commandBufferAllocInfo, &cmdBuffer));

		return cmdBuffer;
	}

	VkFormat VulkanDevice::GetFormatWith(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (auto& format : candidates)
		{
			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &properties);

			if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
			{
				return format;
			}			
		}

		throw std::runtime_error("Failed to identify a supported image format");
	}

	bool VulkanDevice::FormatSupportsLinearFiltering(VkFormat format)
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &formatProperties);

		return formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
	}

	VkCommandPool VulkanDevice::GetOrCreateCommandPool()
	{
		auto threadID = std::this_thread::get_id();
		auto pool = m_CommandPools.find(threadID);

		if (pool != m_CommandPools.end()) return pool->second;

		VkCommandPool newCommandPool;

		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		commandPoolInfo.queueFamilyIndex = m_QueueFamilyIndices.GraphicsIndex.value();

		VulkanUtils::ValidateVkResult(vkCreateCommandPool(m_LogicalDevice, &commandPoolInfo, nullptr, &newCommandPool),
			"Vulkan command pool creation failed");

		m_CommandPools[threadID] = newCommandPool;

		return newCommandPool;
	}

	void VulkanDevice::TargetPhysicalDevice(VkInstance& instance)
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

			if (!pass)
				continue;

			IdentifyQueueFamilies(device, indices);

			if (indices.Complete()) // found a suitable device!
			{
				m_PhysicalDevice = device;
				m_QueueFamilyIndices = indices;

				vkGetPhysicalDeviceFeatures(device, &m_Features);
				vkGetPhysicalDeviceProperties(device, &m_Properties);
				vkGetPhysicalDeviceMemoryProperties(device, &m_MemoryProperties);

				FeedbackToRenderer();

				break;
			}
		}

		if (m_PhysicalDevice == VK_NULL_HANDLE)
		{
			const char* errorMessage = "Failed to identify a GPU meeting the minimum required specifications";
			Z_CORE_CRITICAL(errorMessage);
			throw std::runtime_error(errorMessage);
		}
	}

	bool VulkanDevice::MeetsMinimimumRequirements(const VkPhysicalDevice& device)
	{
		if (!CheckDeviceExtensionSupport(device, s_DeviceExtensions)) return false;

		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(device, &features);
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);
		VkPhysicalDeviceMemoryProperties memory;
		vkGetPhysicalDeviceMemoryProperties(device, &memory);

		const auto& requirements = Application::Get().GetSpecification().GPURequirements;

		if (requirements.IsDiscreteGPU && properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			return false;

		if (requirements.AnisotropicFiltering && features.samplerAnisotropy == VK_FALSE)
			return false;

		if (properties.limits.maxDescriptorSetSampledImages < requirements.MinBoundTextureSlots)
			return false;

		// TODO: add checks for other requirements

		return true;
	}

	bool VulkanDevice::CheckDeviceExtensionSupport(const VkPhysicalDevice& device, const std::vector<const char*>& extensions)
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

	void VulkanDevice::IdentifyQueueFamilies(const VkPhysicalDevice& device, QueueFamilyIndices& indices)
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

	bool VulkanDevice::CheckSwapchainSupport(const VkPhysicalDevice& device, VulkanDeviceSwapchainSupport& support)
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

	void VulkanDevice::QuerySurfaceCapabilities(const VkPhysicalDevice& device, VkSurfaceCapabilitiesKHR& capabilities)
	{
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &capabilities);
	}

	void VulkanDevice::FeedbackToRenderer()
	{
		auto& rendererCapabilities = Renderer::GetGPUCapabilities();
		rendererCapabilities.MaxBoundTextures = m_Properties.limits.maxDescriptorSetSampledImages;
		rendererCapabilities.MaxTextureMultisampling = GetMaxSampleCount();
		rendererCapabilities.DynamicLineWidths = (m_Features.wideLines == VK_TRUE);
		rendererCapabilities.IndependentBlending = (m_Features.independentBlend == VK_TRUE);
	}

	uint32_t VulkanDevice::GetMaxSampleCount()
	{
		VkSampleCountFlags counts = m_Properties.limits.framebufferColorSampleCounts & m_Properties.limits.framebufferDepthSampleCounts;
		VkSampleCountFlagBits decreasingBits[] = {
			VK_SAMPLE_COUNT_64_BIT,
			VK_SAMPLE_COUNT_32_BIT,
			VK_SAMPLE_COUNT_16_BIT,
			VK_SAMPLE_COUNT_8_BIT,
			VK_SAMPLE_COUNT_4_BIT,
			VK_SAMPLE_COUNT_2_BIT,
			VK_SAMPLE_COUNT_1_BIT,
		};

		for (auto& bit : decreasingBits)
		{
			if (counts & bit)
				return VulkanUtils::VkSampleCountFlagBitToSampleCount(bit);
		}
		Z_CORE_ASSERT(false, "Cannot sample at all?");
	}

	VkPhysicalDeviceFeatures VulkanDevice::DesiredFeatures()
	{
		const auto& deviceRequirements = Application::Get().GetSpecification().GPURequirements;
		auto& rendererCapabilities = Renderer::GetGPUCapabilities();

		VkPhysicalDeviceFeatures enabledFeatures{};
		enabledFeatures.samplerAnisotropy = (deviceRequirements.AnisotropicFiltering) ? VK_TRUE : VK_FALSE;
		enabledFeatures.wideLines = (rendererCapabilities.DynamicLineWidths) ? VK_TRUE : VK_FALSE;
		enabledFeatures.independentBlend = (rendererCapabilities.IndependentBlending) ? VK_TRUE : VK_FALSE;

		return enabledFeatures;
	}

}
