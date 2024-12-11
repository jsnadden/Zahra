#include "zpch.h"
#include "VulkanFramebuffer.h"

namespace Zahra
{
	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& specification)
		: m_Specification(specification)
	{
		uint32_t colourAttachmentCount = m_Specification.ColourAttachments.size();
		m_ColourAttachments.resize(colourAttachmentCount);

		for (uint32_t i = 0; i < colourAttachmentCount; i++)
		{
			if (i == 0 && m_Specification.TargetSwapchain)
				CreateAttachmentFromSwapchain();
			else if (const auto& inheritedImage = m_Specification.ColourAttachments[i].InheritFrom)
				m_ColourAttachments.push_back(inheritedImage.As<VulkanImage>());
			else
				CreateAttachmentFromSpecification(i);
		}

		if (m_Specification.HasDepthStencil)
			CreateDepthStencilAttachment();

		CreateVkFramebuffer();
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		
	}

	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
	{

	}

	int VulkanFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		
	}

	void VulkanFramebuffer::CreateAttachmentFromSwapchain()
	{

	}

	void VulkanFramebuffer::CreateAttachmentFromSpecification(uint32_t index)
	{

	}

	void VulkanFramebuffer::CreateDepthStencilAttachment()
	{

	}

	void VulkanFramebuffer::CreateVkFramebuffer()
	{

	}
}
