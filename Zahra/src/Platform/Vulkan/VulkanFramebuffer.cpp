#include "zpch.h"
#include "VulkanFramebuffer.h"

namespace Zahra
{
	namespace VulkanUtils
	{
		static VkAttachmentLoadOp VulkanLoadOp(AttachmentLoadOp loadOp)
		{
			switch (loadOp)
			{
			case AttachmentLoadOp::Unspecified:		return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			case AttachmentLoadOp::Clear:			return VK_ATTACHMENT_LOAD_OP_CLEAR;
			case AttachmentLoadOp::Load:			return VK_ATTACHMENT_LOAD_OP_LOAD;
			}
			Z_CORE_ASSERT(false, "Unsupported load operation");
			return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
		}

		static VkImageLayout InitialLayoutFromLoadOp(AttachmentLoadOp loadOp, bool depthStencil)
		{
			if (loadOp == AttachmentLoadOp::Clear)
				return VK_IMAGE_LAYOUT_UNDEFINED;
			else if (depthStencil)
				return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; // TODO: framebuffer spec should flag whether we'll sample the depth/stencil attachment or not
			else
				return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}

	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& specification)
		: m_Specification(specification)
	{
		Z_CORE_ASSERT(ValidateSpecification());

		if (m_Specification.Width == 0)
			m_Specification.Width = Renderer::GetSwapchainWidth();

		if (m_Specification.Height == 0)
			m_Specification.Height = Renderer::GetSwapchainHeight();

		CreateAttachments();
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		DestroyAttachments();
	}

	const Ref<Image2D>& VulkanFramebuffer::GetColourAttachment(uint32_t index) const
	{
		Z_CORE_ASSERT(index < m_ColourAttachments.size(), "Invalid attachment index");
		return m_ColourAttachments[index];
	}

	const Ref<Image2D>& VulkanFramebuffer::GetDepthStencilAttachment() const
	{
		return m_DepthStencilAttachment;
	}

	std::vector<VkImageView> const VulkanFramebuffer::GetImageViews()
	{
		std::vector<VkImageView> imageViews;

		for (auto& attachment : m_ColourAttachments)
			imageViews.push_back(attachment->GetVkImageView());

		if (m_Specification.HasDepthStencil)
			imageViews.push_back(m_DepthStencilAttachment->GetVkImageView());

		return imageViews;
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

	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		m_Specification.Width = width;
		m_Specification.Height = height;

		for (uint32_t i = 0; i < m_ColourAttachments.size(); i++)
		{
			if (auto& inheritedImage = m_Specification.ColourAttachmentSpecs[i].InheritFrom)
			{
				Z_CORE_ASSERT(inheritedImage->GetWidth() == width && inheritedImage->GetHeight() == height,
					"A framebuffer should only be resized after all images its attachments inherit from have already been resized");

				continue;
			}

			m_ColourAttachments[i]->Resize(width, height);
		}

		if (auto& inheritedImage = m_Specification.DepthStencilAttachmentSpec.InheritFrom)
		{
			Z_CORE_ASSERT(inheritedImage->GetWidth() == width && inheritedImage->GetHeight() == height,
				"A framebuffer should only be resized after all images its attachments inherit from have already been resized");

			return;
		}

		m_DepthStencilAttachment->Resize(width, height);
	}

	void VulkanFramebuffer::CreateAttachments()
	{
		uint32_t colourAttachmentCount = m_Specification.ColourAttachmentSpecs.size();
		m_ColourAttachments.resize(colourAttachmentCount);

		for (uint32_t i = 0; i < colourAttachmentCount; i++)
		{
			if (m_Specification.ColourAttachmentSpecs[i].InheritFrom)
				m_ColourAttachments[i] = m_Specification.ColourAttachmentSpecs[i].InheritFrom.As<VulkanImage2D>();
			else
				CreateColourAttachment(i);

			// TODO: for now we're setting up all attachments to be used as textures, which may be less-than-optimal

			auto& description = m_AttachmentDescriptions.emplace_back();
			description.flags = 0;
			description.format = m_ColourAttachments[i]->GetVkFormat();
			description.samples = VK_SAMPLE_COUNT_1_BIT;
			description.loadOp = VulkanUtils::VulkanLoadOp(m_Specification.ColourAttachmentSpecs[i].LoadOp);
			description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			description.initialLayout = VulkanUtils::InitialLayoutFromLoadOp(m_Specification.ColourAttachmentSpecs[i].LoadOp, false);
			description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		if (m_Specification.HasDepthStencil)
		{
			if (m_Specification.DepthStencilAttachmentSpec.InheritFrom)
				m_DepthStencilAttachment = m_Specification.DepthStencilAttachmentSpec.InheritFrom.As<VulkanImage2D>();
			else
				CreateDepthStencilAttachment();

			auto& description = m_AttachmentDescriptions.emplace_back();
			description.flags = 0;
			description.format = VulkanUtils::GetSupportedDepthStencilFormat();
			description.samples = VK_SAMPLE_COUNT_1_BIT;
			description.loadOp = VulkanUtils::VulkanLoadOp(m_Specification.DepthStencilAttachmentSpec.LoadOp);
			description.storeOp = VK_ATTACHMENT_STORE_OP_STORE; 
			description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			description.initialLayout = VulkanUtils::InitialLayoutFromLoadOp(m_Specification.DepthStencilAttachmentSpec.LoadOp, true);
			description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; // TODO: should depend on a "sampled" flag in spec
		}			
	}

	void VulkanFramebuffer::DestroyAttachments()
	{
		m_ColourAttachments.clear();
		m_DepthStencilAttachment.Reset();
	}

	void VulkanFramebuffer::CreateColourAttachment(uint32_t index)
	{
		uint32_t framesInFlight = Renderer::GetFramesInFlight();

		ImageSpecification imageSpec{};
		imageSpec.Width = m_Specification.Width;
		imageSpec.Height = m_Specification.Height;
		imageSpec.Format = m_Specification.ColourAttachmentSpecs[index].Format;
		imageSpec.Sampled = true; // for now

		m_ColourAttachments[index] = Ref<VulkanImage2D>::Create(imageSpec);
	}

	void VulkanFramebuffer::CreateDepthStencilAttachment()
	{
		uint32_t framesInFlight = Renderer::GetFramesInFlight();

		ImageSpecification imageSpec{};
		imageSpec.Width = m_Specification.Width;
		imageSpec.Height = m_Specification.Height;
		imageSpec.Format = ImageFormat::DepthStencil;
		imageSpec.Sampled = true; // for now

		m_DepthStencilAttachment = Ref<VulkanImage2D>::Create(imageSpec);
	}

	bool VulkanFramebuffer::ValidateSpecification()
	{
		// TODO: check attachment specs are sensible
		return true;
	}
}