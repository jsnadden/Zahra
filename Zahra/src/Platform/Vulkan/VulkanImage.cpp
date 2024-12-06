#include "zpch.h"
#include "VulkanImage.h"

namespace Zahra
{
	VulkanImage::VulkanImage(uint32_t width, uint32_t height, ImageFormat format, ImageUsage usage)
		: m_Usage(VulkanUtils::VulkanImageUsage(usage)), m_Dimensions({ width, height })
	{
		m_Format = usage == ImageUsage::DepthStencilAttachment ?
			VulkanUtils::GetSupportedDepthStencilFormat() : VulkanUtils::GetColourFormat(format);

		VkImageAspectFlags aspect = VulkanUtils::VulkanImageAspect(usage);
		auto device = VulkanContext::GetCurrentDevice();			

		device->CreateVulkanImage(width, height, m_Format,
			VK_IMAGE_TILING_OPTIMAL, m_Usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_Image, m_Memory);

		m_ImageView = device->CreateVulkanImageView(m_Format, m_Image, aspect);
	}

	VulkanImage::~VulkanImage()
	{
		// TODO: employ VMA

		auto& device = VulkanContext::GetCurrentVkDevice();

		vkDestroyImageView(device, m_ImageView, nullptr);
		m_ImageView = VK_NULL_HANDLE;

		vkFreeMemory(device, m_Memory, nullptr);
		m_Memory = VK_NULL_HANDLE;

		vkDestroyImage(device, m_Image, nullptr);
		m_Image = VK_NULL_HANDLE;
	}

	void VulkanImage::TransitionLayout(VkImageLayout newLayout)
	{
		if (m_CurrentLayout == newLayout)
			return;

		VulkanContext::GetCurrentDevice()->TransitionVulkanImageLayout(m_Image, m_Format, m_CurrentLayout, newLayout);

		m_CurrentLayout = newLayout;

	}

	void VulkanImage::SetData(const VkBuffer& srcBuffer)
	{
		VulkanContext::GetCurrentDevice()->CopyVulkanBufferToImage(srcBuffer, m_Image, m_Dimensions.width, m_Dimensions.height);
	}

}
