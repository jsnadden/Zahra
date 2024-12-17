#include "zpch.h"
#include "VulkanDevice.h"

#include "Platform/Vulkan/VulkanUtils.h"
#include "Zahra/Core/Application.h"

namespace Zahra
{

	namespace VulkanUtils
	{
		static bool FormatHasStencilComponent(VkFormat format)
		{
			return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
		}
	}

	VulkanDevice::~VulkanDevice()
	{
		Shutdown();
	}

	void VulkanDevice::Shutdown()
	{
		for (auto& [threadID, pool] : m_CommandPools)
			vkDestroyCommandPool(m_LogicalDevice, pool, nullptr);
		m_CommandPools.clear();

		if (m_LogicalDevice) vkDestroyDevice(m_LogicalDevice, nullptr);
		m_LogicalDevice = VK_NULL_HANDLE;
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

		EndTemporaryCommandBuffer(commandBuffer);
	}

	void VulkanDevice::CreateVulkanImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1; // TODO: mipmapping
		imageInfo.arrayLayers = 1; // TODO: can be used for e.g. shadow map cascades
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: MSAA
		
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

	VkImageView VulkanDevice::CreateVulkanImageView(VkFormat format, VkImage& image, VkImageAspectFlags aspectFlags)
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
		viewInfo.subresourceRange.levelCount = 1; // TODO: mipmapping
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VulkanUtils::ValidateVkResult(vkCreateImageView(m_LogicalDevice, &viewInfo, nullptr, &imageView),
			"Vulkan image view creation failed");

		return imageView;
	}

	VkSampler VulkanDevice::CreateVulkanImageSampler(VkFilter minFilter, VkFilter magFilter, VkSamplerAddressMode addressMode, VkSamplerMipmapMode mipmapMode)
	{
		VkSampler sampler;

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = magFilter;
		samplerInfo.minFilter = minFilter;
		samplerInfo.mipmapMode = mipmapMode;
		samplerInfo.addressModeU = addressMode;
		samplerInfo.addressModeV = addressMode;
		samplerInfo.addressModeW = addressMode;
		samplerInfo.anisotropyEnable = VK_TRUE; // TODO: get this from the application's graphics settings
		samplerInfo.maxAnisotropy = m_Properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f; // TODO: mipmapping

		VulkanUtils::ValidateVkResult(vkCreateSampler(m_LogicalDevice, &samplerInfo, nullptr, &sampler),
			"Vulkan image sampler creation failed");

		return sampler;
	}

	void VulkanDevice::CopyVulkanBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = GetTemporaryCommandBuffer();

		VkBufferImageCopy copyInfo{};
		copyInfo.bufferOffset = 0;
		copyInfo.bufferRowLength = 0;
		copyInfo.bufferImageHeight = 0;
		copyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyInfo.imageSubresource.mipLevel = 0; // TODO: mipmapping
		copyInfo.imageSubresource.baseArrayLayer = 0;
		copyInfo.imageSubresource.layerCount = 1;
		copyInfo.imageOffset = { 0, 0, 0 };
		copyInfo.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer, buffer, image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo); // this assumes we've already transitioned the image layout to TRANSFER_DST_OPTIMAL;

		EndTemporaryCommandBuffer(commandBuffer);

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

		EndTemporaryCommandBuffer(commandBuffer);
	}

	void VulkanDevice::TransitionVulkanImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
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
		barrier.subresourceRange.baseMipLevel = 0; // TODO: configure mipmapping
		barrier.subresourceRange.levelCount = 1; // TODO: configure mipmapping
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

		EndTemporaryCommandBuffer(commandBuffer);
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

	void VulkanDevice::EndTemporaryCommandBuffer(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_GraphicsQueue);
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

	VkFormat VulkanDevice::CheckFormatSupport(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
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

}
