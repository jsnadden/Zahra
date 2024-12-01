#include "zpch.h"
#include "VulkanAttachment.h"

namespace Zahra
{
	VulkanAttachment::VulkanAttachment(VkExtent2D size, VkFormat format, VkImageUsageFlags usage)
		: m_Format(format)
	{
		auto device = VulkanContext::GetCurrentDevice();
		device->CreateVulkanImage(size.width, size.height, m_Format,
			VK_IMAGE_TILING_OPTIMAL, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_Image, m_Memory);

		VkImageAspectFlags aspect;
		if (VulkanUtils::IsDepthFormat(format) && (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		else if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		else
			Z_CORE_ASSERT(false, "Unsupported combination of VkFormat + VkImageUsageFlags");

		m_ImageView = device->CreateVulkanImageView(m_Format, m_Image, aspect);
	}

	VulkanAttachment::~VulkanAttachment()
	{
		auto& device = VulkanContext::GetCurrentVkDevice();

		vkDestroyImageView(device, m_ImageView, nullptr);
		m_ImageView = VK_NULL_HANDLE;

		vkFreeMemory(device, m_Memory, nullptr);
		m_Memory = VK_NULL_HANDLE;

		vkDestroyImage(device, m_Image, nullptr);
		m_Image = VK_NULL_HANDLE;
	}

}
