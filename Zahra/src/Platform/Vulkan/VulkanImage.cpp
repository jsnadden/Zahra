#include "zpch.h"
#include "VulkanImage.h"

namespace Zahra
{
	VulkanImage::VulkanImage(ImageSpecification specification)
		: m_Specification(specification)
	{
		m_Format = specification.Usage == ImageUsage::DepthStencilAttachment ?
			VulkanUtils::GetSupportedDepthStencilFormat() : VulkanUtils::GetColourFormat(specification.Format);

		m_Usage = VulkanUtils::VulkanImageUsage(specification.Usage);

		VkImageAspectFlags aspect = VulkanUtils::VulkanImageAspect(specification.Usage);
		auto device = VulkanContext::GetCurrentDevice();			

		device->CreateVulkanImage(specification.Width, specification.Height, m_Format,
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

	//void VulkanImage::CopyData(Ref<Image>& source)
	//{
	//	Ref<VulkanImage> srcImage = source.As<VulkanImage>();

	//	Z_CORE_ASSERT(source->GetWidth() == m_Dimensions.width && source->GetHeight() == m_Dimensions.height, "Cannot copy images if their dimensions do not match");

	//	Ref<VulkanDevice> device = VulkanContext::GetCurrentDevice();

	//	device->CopyVulkanImage(srcImage->m_Image, m_Image, m_Dimensions.width, m_Dimensions.height);
	//}

	void VulkanImage::TransitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VulkanContext::GetCurrentDevice()->TransitionVulkanImageLayout(m_Image, m_Format, oldLayout, newLayout);
	}

	void VulkanImage::SetData(const VkBuffer& srcBuffer)
	{
		VulkanContext::GetCurrentDevice()->CopyVulkanBufferToImage(srcBuffer, m_Image, m_Specification.Width, m_Specification.Height);
	}

}
