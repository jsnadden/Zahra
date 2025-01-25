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

		//TransitionLayout(ImageLayout::Unspecified, m_Specification.InitialLayout);

		if (m_Specification.CreatePixelBuffer)
			CreatePixelBuffer();
	}

	void VulkanImage2D::Cleanup()
	{
		auto& device = VulkanContext::GetCurrentVkDevice();
		vkDeviceWaitIdle(device);

		if (m_Specification.CreatePixelBuffer)
		{
			vkUnmapMemory(device, m_PixelBufferMemory);
			vkDestroyBuffer(device, m_PixelBuffer, nullptr);
			vkFreeMemory(device, m_PixelBufferMemory, nullptr);
			m_PixelBufferMappedAddress = nullptr;
		}

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
		Z_CORE_ASSERT(m_CurrentLayout == ImageLayout::Unspecified, "This method should be called before any other image layout transitions have taken place");

		bool isDepthStencil = (m_Specification.Format == ImageFormat::DepthStencil);
		VkImageAspectFlags aspectMask = isDepthStencil ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		auto& device = VulkanContext::GetCurrentDevice();
		VkCommandBuffer commandBuffer = device->GetTemporaryCommandBuffer();

		// This method requires a number of layout transitions
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = m_Image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = aspectMask;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		////////////////////////////////////////////////////////////////////////////////////
		// Transition all mip levels to transfer destination layout
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = m_Specification.MipLevels;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

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
		// Generate mips
		if (m_Specification.MipLevels > 1)
		{
			barrier.subresourceRange.levelCount = 1; // only dealing with one level at a time

			// TODO: figure out how to generate mips without linear filtering
			Z_CORE_ASSERT(device->FormatSupportsLinearFiltering(VulkanUtils::VulkanFormat(m_Specification.Format)),
				"Currently only supporting mipmapping for texture formats supporting linear filtering")

			int32_t levelWidth = (int32_t)m_Specification.Width;
			int32_t levelHeight = (int32_t)m_Specification.Height;

			VkImageBlit blit{};
			blit.srcSubresource.aspectMask = blit.dstSubresource.aspectMask = aspectMask;
			blit.srcSubresource.baseArrayLayer = blit.dstSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = blit.dstSubresource.layerCount = 1;
			blit.srcOffsets[0] = blit.dstOffsets[0] = { 0, 0, 0 };			

			for (uint32_t level = 1; level < m_Specification.MipLevels; level++)
			{
				int32_t nextWidth = levelWidth > 1 ? levelWidth / 2 : 1;
				int32_t nextHeight = levelHeight > 1 ? levelHeight / 2 : 1;

				////////////////////////////////////////////////////////////////////////////////////
				// Transition previous mip level to transfer source layout
				barrier.subresourceRange.baseMipLevel = level-1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

				////////////////////////////////////////////////////////////////////////////////////
				// Copy and downscale image from previous mip level
				blit.srcSubresource.mipLevel = level - 1;
				blit.dstSubresource.mipLevel = level;
				blit.srcOffsets[1] = { levelWidth, levelHeight, 1 };
				blit.dstOffsets[1] = { nextWidth, nextHeight, 1 };

				vkCmdBlitImage(commandBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

				////////////////////////////////////////////////////////////////////////////////////
				// Transition previous mip level to sampleable layout
				barrier.subresourceRange.baseMipLevel = level - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = isDepthStencil ?
					VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL :
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = isDepthStencil ?
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT :
					VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, isDepthStencil ?
					VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					0, 0, nullptr, 0, nullptr, 1, &barrier);

				levelWidth = nextWidth;
				levelHeight = nextHeight;
			}
		}

		////////////////////////////////////////////////////////////////////////////////////
		// Transition final mip level to sampleable layout
		barrier.subresourceRange.baseMipLevel = m_Specification.MipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = isDepthStencil ?
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL :
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = isDepthStencil ?
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT :
			VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, isDepthStencil ?
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

		m_CurrentLayout = isDepthStencil ?
			ImageLayout::DepthStencilAttachment :
			ImageLayout::ColourAttachment;

		device->SubmitTemporaryCommandBuffer(commandBuffer);
	}

	void* VulkanImage2D::ReadPixel(int32_t x, int32_t y)
	{
		VulkanContext::GetCurrentDevice()->CopyPixelToBuffer(m_Image, m_PixelBuffer, x, y);
		return m_PixelBufferMappedAddress;
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

		device->CreateVulkanImage(m_Specification.Width, m_Specification.Height, m_Specification.MipLevels,
			VulkanUtils::VulkanFormat(m_Specification.Format), VK_IMAGE_TILING_OPTIMAL,
			usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_Memory);
	}

	void VulkanImage2D::GenerateMips()
	{

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
		
		m_ImageView = device->CreateVulkanImageView(format, m_Image, aspectMask, m_Specification.MipLevels);
	}

	void VulkanImage2D::CreateSampler()
	{
		auto& device = VulkanContext::GetCurrentDevice();
		bool linearFiltering = device->FormatSupportsLinearFiltering(VulkanUtils::VulkanFormat(m_Specification.Format));

		VkFilter filter;
		VkSamplerMipmapMode mipmapMode;

		if (linearFiltering)
		{
			filter = VK_FILTER_LINEAR;
			mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		}
		else
		{
			filter = VK_FILTER_NEAREST;
			mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		}

		// TODO: get other parameters from image spec, renderer configuration OR runtime graphics settings
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = filter;
		samplerInfo.minFilter = filter;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = device->GetDeviceProperties().limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = mipmapMode;
		samplerInfo.minLod = .0f;
		samplerInfo.maxLod = (float)m_Specification.MipLevels;
		samplerInfo.mipLodBias = .0f;

		VulkanUtils::ValidateVkResult(vkCreateSampler(device->GetVkDevice(), &samplerInfo, nullptr, &m_Sampler),
			"Vulkan image sampler creation failed");
	}

	void VulkanImage2D::CreatePixelBuffer()
	{
		auto& device = VulkanContext::GetCurrentDevice();

		VkDeviceSize pixelSize = Image::BytesPerPixel(m_Specification.Format);

		device->CreateVulkanBuffer(pixelSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_PixelBuffer, m_PixelBufferMemory);

		vkMapMemory(device->GetVkDevice(), m_PixelBufferMemory, 0, pixelSize, 0, &m_PixelBufferMappedAddress);
	}

	//void VulkanImage2D::TransitionLayout(ImageLayout from, ImageLayout to)
	//{
	//	if (from == to)
	//		return;

	//	bool isDepthStencil = (m_Specification.Format == ImageFormat::DepthStencil);
	//	VkImageAspectFlags aspectMask = isDepthStencil ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	//	auto& device = VulkanContext::GetCurrentDevice();
	//	VkCommandBuffer commandBuffer = device->GetTemporaryCommandBuffer();

	//	VkAccessFlags srcAccessMask = VulkanUtils::AccessFlagForLayoutTransition(from);
	//	VkPipelineStageFlags srcStageMask = VulkanUtils::StageFlagForLayoutTransition(from);

	//	VkAccessFlags dstAccessMask = VulkanUtils::AccessFlagForLayoutTransition(to);
	//	VkPipelineStageFlags dstStageMask = VulkanUtils::StageFlagForLayoutTransition(to);

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
	//		barrier.srcAccessMask = srcAccessMask;
	//		barrier.dstAccessMask = dstAccessMask;
	//	}

	//	vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	//	device->SubmitTemporaryCommandBuffer(commandBuffer); //, GPUQueueType::Transfer);
	//}


}
