#include "zpch.h"
#include "VulkanImage.h"

namespace Zahra
{
	VulkanImage::VulkanImage(ImageSpecification specification)
		: m_Specification(specification)
	{
		InitData();

		VulkanContext::GetCurrentDevice()->CreateVulkanImage(specification.Width, specification.Height, m_Format,
			VK_IMAGE_TILING_OPTIMAL, m_Usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_Image, m_Memory);

		VulkanContext::GetCurrentDevice()->TransitionVulkanImageLayout(m_Image, m_Format,
			VK_IMAGE_LAYOUT_UNDEFINED, VulkanUtils::VulkanImageLayout(m_Specification.Layout));

		CreateImageView();
	}

	VulkanImage::VulkanImage(VkImage image, VkDeviceMemory memory, ImageSpecification specification)
		: m_Specification(specification)
	{
		InitData();

		m_Image = image;
		m_Memory = memory;

		CreateImageView();
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

	/*void VulkanImage::CopyData(Ref<Image>& source)
	{
		Ref<VulkanImage> srcImage = source.As<VulkanImage>();

		Z_CORE_ASSERT(source->GetWidth() == m_Specification.Width && source->GetHeight() == m_Specification.Height, "Cannot copy images if their dimensions do not match");

		Ref<VulkanDevice> device = VulkanContext::GetCurrentDevice();

		device->CopyVulkanImage(srcImage->m_Image, m_Image, m_Specification.Width, m_Specification.Height);
	}*/

	void VulkanImage::TransitionLayout(ImageLayout layout)
	{
		VkImageLayout oldLayout = VulkanUtils::VulkanImageLayout(m_Specification.Layout);
		VkImageLayout newLayout = VulkanUtils::VulkanImageLayout(layout);

		VulkanContext::GetCurrentDevice()->TransitionVulkanImageLayout(m_Image, m_Format, oldLayout, newLayout);
		m_Specification.Layout = layout;
	}

	void VulkanImage::SetData(const VkBuffer& srcBuffer)
	{
		VulkanContext::GetCurrentDevice()->CopyVulkanBufferToImage(srcBuffer, m_Image, m_Specification.Width, m_Specification.Height);
	}

	void VulkanImage::InitData()
	{
		m_Format = m_Specification.Usage == ImageUsage::DepthStencilAttachment ?
			VulkanUtils::GetSupportedDepthStencilFormat() : VulkanUtils::VulkanColourFormat(m_Specification.Format);

		m_Usage = VulkanUtils::VulkanImageUsage(m_Specification.Usage);
	}

	void VulkanImage::CreateImageView()
	{
		m_ImageView = VulkanContext::GetCurrentDevice()->CreateVulkanImageView(m_Format, m_Image, VulkanUtils::VulkanImageAspect(m_Specification.Usage));
	}

}
