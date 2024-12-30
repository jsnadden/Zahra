#include "zpch.h"
#include "VulkanImage.h"

namespace Zahra
{
	VulkanImage2D::VulkanImage2D(Image2DSpecification specification)
		: m_Specification(specification)
	{
		Init();
	}

	VulkanImage2D::~VulkanImage2D()
	{
		Cleanup();
	}

	void VulkanImage2D::Init()
	{
		CreateAndAllocateImage();
		CreateImageView();

		if (m_Specification.Sampled)
			CreateSampler();

		/*if (m_Specification.InitialLayout != ImageLayout::Unspecified)
			TransitionLayout();*/
	}

	void VulkanImage2D::Cleanup()
	{
		auto& device = VulkanContext::GetCurrentVkDevice();
		vkDeviceWaitIdle(device);

		vkDestroySampler(device, m_Sampler, nullptr);
		vkDestroyImageView(device, m_ImageView, nullptr);
		vkFreeMemory(device, m_Memory, nullptr);
		vkDestroyImage(device, m_Image, nullptr);
	}

	void VulkanImage2D::Resize(uint32_t width, uint32_t height)
	{
		if (width > 0 && height > 0)
		{
			Cleanup();

			m_Specification.Width = width;
			m_Specification.Height = height;

			Init();
		}
	}

	void VulkanImage2D::SetData(const VkBuffer& srcBuffer)
	{
		Z_CORE_ASSERT(m_Specification.Sampled, "This method should only be called for texture creation");
		Z_CORE_ASSERT(m_CurrentLayout == VK_IMAGE_LAYOUT_UNDEFINED, "This method should only be called before other image layout transitions have taken place");

		bool isDepthStencil = (m_Specification.Format == ImageFormat::DepthStencil);
		VkImageAspectFlags aspectMask = isDepthStencil ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		auto& device = VulkanContext::GetCurrentDevice();
		VkCommandBuffer commandBuffer = device->GetTemporaryCommandBuffer();

		////////////////////////////////////////////////////////////////////////////////////
		// Transition layout to transfer destination
		VkImageMemoryBarrier barrier{};
		{
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // not transferring ownership
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_Image;
			barrier.subresourceRange.aspectMask = aspectMask;
			barrier.subresourceRange.baseMipLevel = 0; // TODO: configure mipmapping
			barrier.subresourceRange.levelCount = 1; // TODO: configure mipmapping
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		}

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		////////////////////////////////////////////////////////////////////////////////////
		// Copy buffer data to image
		VkBufferImageCopy copyInfo{};
		{
			copyInfo.bufferOffset = 0;
			copyInfo.bufferRowLength = 0;
			copyInfo.bufferImageHeight = 0;
			copyInfo.imageSubresource.aspectMask = aspectMask;
			copyInfo.imageSubresource.mipLevel = 0; // TODO: mipmapping
			copyInfo.imageSubresource.baseArrayLayer = 0;
			copyInfo.imageSubresource.layerCount = 1;
			copyInfo.imageOffset = { 0, 0, 0 };
			copyInfo.imageExtent = { m_Specification.Width, m_Specification.Height, 1 };
		}

		vkCmdCopyBufferToImage(commandBuffer, srcBuffer, m_Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

		////////////////////////////////////////////////////////////////////////////////////
		// Transition to texture layout
		{
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = isDepthStencil ?
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL :
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_Image;
			barrier.subresourceRange.aspectMask = aspectMask;
			barrier.subresourceRange.baseMipLevel = 0; // TODO: configure mipmapping
			barrier.subresourceRange.levelCount = 1; // TODO: configure mipmapping
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = isDepthStencil ?
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT :
			VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, isDepthStencil ?
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

		device->EndTemporaryCommandBuffer(commandBuffer);

		m_CurrentLayout = isDepthStencil ?
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL :
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	void VulkanImage2D::CreateAndAllocateImage()
	{
		auto& device = VulkanContext::GetCurrentDevice();

		VkImageUsageFlags usage = 0;
		{
			if (m_Specification.Format == ImageFormat::DepthStencil)
				usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			else
				usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			if (m_Specification.Sampled)
				usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

			if (m_Specification.TransferDestination)
				usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			if (m_Specification.TransferSource)
				usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}

		device->CreateVulkanImage(m_Specification.Width, m_Specification.Height,
			VulkanUtils::VulkanFormat(m_Specification.Format), VK_IMAGE_TILING_OPTIMAL,
			usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_Memory);
	}

	void VulkanImage2D::CreateImageView()
	{
		auto& device = VulkanContext::GetCurrentDevice();

		VkFormat format = VulkanUtils::VulkanFormat(m_Specification.Format);
		
		VkImageAspectFlags aspectMask;
		{
			if (m_Specification.Format == ImageFormat::DepthStencil)
				aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			else
				aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		
		m_ImageView = device->CreateVulkanImageView(format, m_Image, aspectMask);
	}

	void VulkanImage2D::CreateSampler()
	{
		auto& device = VulkanContext::GetCurrentDevice();

		VkFilter filter;
		VkSamplerMipmapMode mipmapMode;

		if (VulkanUtils::IsIntegerFormat(m_Specification.Format))
		{
			filter = VK_FILTER_NEAREST;
			mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		}
		else
		{
			filter = VK_FILTER_LINEAR;
			mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		}
		// TODO: set "address mode" in image/texture spec
		m_Sampler = device->CreateVulkanImageSampler(filter, filter, VK_SAMPLER_ADDRESS_MODE_REPEAT, mipmapMode);
	}

	//void VulkanImage2D::TransitionLayout()
	//{
	//	bool isDepthStencil = (m_Specification.Format == ImageFormat::DepthStencil);
	//	VkImageAspectFlags aspectMask = isDepthStencil ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	//	auto& device = VulkanContext::GetCurrentDevice();
	//	VkCommandBuffer commandBuffer = device->GetTemporaryCommandBuffer();

	//	VkAccessFlags dstAccessMask = VulkanUtils::DstAccessForLayoutTransition(m_Specification.InitialLayout);
	//	VkPipelineStageFlags dstStageMask = VulkanUtils::DstStageForLayoutTransition(m_Specification.InitialLayout);

	//	VkImageMemoryBarrier barrier{};
	//	{
	//		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	//		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//		barrier.newLayout = VulkanUtils::VulkanImageLayout(m_Specification.InitialLayout);
	//		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // not transferring ownership
	//		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//		barrier.image = m_Image;
	//		barrier.subresourceRange.aspectMask = aspectMask;
	//		barrier.subresourceRange.baseMipLevel = 0; // TODO: configure mipmapping
	//		barrier.subresourceRange.levelCount = 1; // TODO: configure mipmapping
	//		barrier.subresourceRange.baseArrayLayer = 0;
	//		barrier.subresourceRange.layerCount = 1;
	//		barrier.srcAccessMask = 0;
	//		barrier.dstAccessMask = dstAccessMask;
	//	}

	//	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	//		dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	//	device->EndTemporaryCommandBuffer(commandBuffer);
	//}

}
