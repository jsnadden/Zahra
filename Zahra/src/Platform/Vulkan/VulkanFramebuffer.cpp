#include "zpch.h"
#include "VulkanFramebuffer.h"

namespace Zahra
{
	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& specification)
		: m_Specification(specification)
	{
		CreateAttachments();
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		DestroyAttachments();
	}

	/*int VulkanFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		
	}*/

	const Ref<Image>& VulkanFramebuffer::GetColourAttachment(uint32_t index) const
	{
		return m_ColourAttachments[index][Renderer::GetCurrentFrameIndex()];
	}

	const Ref<Image>& VulkanFramebuffer::GetDepthStencilAttachment() const
	{
		return m_DepthStencilAttachment[Renderer::GetCurrentFrameIndex()];
	}

	std::vector<VkClearValue> const VulkanFramebuffer::GetClearValues()
	{
		std::vector<VkClearValue> clearValues;

		for (auto& attachment : m_Specification.ColourAttachmentSpecs)
		{
			VkClearValue colour = { attachment.ClearColour.r, attachment.ClearColour.g, attachment.ClearColour.b, 1.0f };
			clearValues.push_back(colour);
		}

		if (m_Specification.HasDepthStencil)
		{
			VkClearValue depthStencil = { 1.0f, 0 };
			clearValues.push_back(depthStencil);
		}

		return clearValues;
	}

	std::vector<VkFramebuffer> VulkanFramebuffer::GenerateVkFramebuffers(const VkRenderPass& renderPass)
	{
		std::vector<VkFramebuffer> framebuffers;
		VkDevice& device = VulkanContext::GetCurrentVkDevice();

		uint32_t framesInFlight = Renderer::GetFramesInFlight();
		for (uint32_t frame = 0; frame < framesInFlight; frame++)
		{
			auto& fb = framebuffers.emplace_back();

			std::vector<VkImageView> attachmentImageViews;

			for (auto& attachment : m_ColourAttachments)
			{	
				if (attachment.empty()) // should only be true for swapchain-targeting attachments (verify this)
					attachmentImageViews.emplace_back(m_SwapchainImageViews[frame]);
				else
					attachmentImageViews.emplace_back(attachment[frame]->GetVkImageView());
			}

			if (m_Specification.HasDepthStencil)
				attachmentImageViews.emplace_back(m_DepthStencilAttachment[frame]->GetVkImageView());

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = attachmentImageViews.size();
			framebufferInfo.pAttachments = attachmentImageViews.data();
			framebufferInfo.width = m_Specification.Width;
			framebufferInfo.height = m_Specification.Height;
			framebufferInfo.layers = 1;

			VulkanUtils::ValidateVkResult(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &fb),
				"Vulkan framebuffer creation failed");
		}

		return framebuffers;
	}

	void VulkanFramebuffer::CreateAttachments()
	{
		uint32_t colourAttachmentCount = m_Specification.ColourAttachmentSpecs.size();
		uint32_t framesInFlight = Renderer::GetFramesInFlight();
		m_ColourAttachments.resize(colourAttachmentCount);

		for (uint32_t i = 0; i < colourAttachmentCount; i++)
		{
			if (i == 0 && m_Specification.TargetSwapchain)
			{
				m_SwapchainImageViews = VulkanContext::Get()->GetSwapchain()->GetSwapchainImageViews();
			}
			else if (const auto& inheritedImage = m_Specification.ColourAttachmentSpecs[i].InheritFrom)
			{
				for (uint32_t frame = 0; frame < framesInFlight; frame++)
					m_ColourAttachments[i][frame] = inheritedImage.As<VulkanImage>();
			}
			else
			{
				CreateColourAttachmentFromSpecification(i);
			}
		}

		if (m_Specification.HasDepthStencil)
			CreateDepthStencilAttachment();
	}

	void VulkanFramebuffer::DestroyAttachments()
	{
		m_ColourAttachments.clear();
		m_DepthStencilAttachment.clear();
	}

	void VulkanFramebuffer::CreateColourAttachmentFromSpecification(uint32_t index)
	{
		uint32_t framesInFlight = Renderer::GetFramesInFlight();

		ImageSpecification imageSpec{};
		imageSpec.Width = m_Specification.Width;
		imageSpec.Height = m_Specification.Height;
		imageSpec.Format = m_Specification.ColourAttachmentSpecs[index].Format;
		imageSpec.Usage = ImageUsage::ColourAttachment;
		imageSpec.Layout = m_Specification.ColourAttachmentSpecs[index].InitialLayout;

		for (uint32_t frame = 0; frame < framesInFlight; frame++)
			m_ColourAttachments[index].emplace_back(Ref<VulkanImage>::Create(imageSpec));
	}

	void VulkanFramebuffer::CreateDepthStencilAttachment()
	{
		uint32_t framesInFlight = Renderer::GetFramesInFlight();

		ImageSpecification imageSpec{};
		imageSpec.Width = m_Specification.Width;
		imageSpec.Height = m_Specification.Height;
		imageSpec.Usage = ImageUsage::DepthStencilAttachment;
		imageSpec.Layout = m_Specification.DepthStencilAttachmentSpec.InitialLayout;

		for (uint32_t frame = 0; frame < framesInFlight; frame++)
			m_DepthStencilAttachment.emplace_back(Ref<VulkanImage>::Create(imageSpec));
	}
}
