#pragma once

#include "Platform/Vulkan/VulkanImage.h"
#include "Zahra/Renderer/Framebuffer.h"


namespace Zahra
{
	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const FramebufferSpecification& specification);
		~VulkanFramebuffer();

		virtual const FramebufferSpecification& GetSpecification() const { return m_Specification; }

		virtual Ref<Image2D> GetColourAttachment(uint32_t index) const;
		virtual Ref<Image2D> GetDepthStencilAttachment() const;

		virtual uint32_t GetColourAttachmentCount() const override { return m_ColourAttachmentCount; }

		virtual uint32_t GetWidth() const override { return m_Specification.Width; }
		virtual uint32_t GetHeight() const override { return m_Specification.Height; }

		std::vector<VkImageView> const GetImageViews();
		std::vector<VkClearValue> const GetClearValues();
		const std::vector<VkAttachmentDescription>& GetAttachmentDescriptions() const { return m_AttachmentDescriptions; }

		virtual void Resize(uint32_t width, uint32_t height) override;

	private:
		FramebufferSpecification m_Specification;

		uint32_t m_ColourAttachmentCount = 0;
		std::vector<Ref<VulkanImage2D>> m_ColourAttachments;
		Ref<VulkanImage2D> m_DepthStencilAttachment;

		std::vector<VkAttachmentDescription> m_AttachmentDescriptions;
		
		void CreateAttachments();
		void DestroyAttachments();

		void CreateColourAttachment(uint32_t index);
		void CreateDepthStencilAttachment();

		bool SpecificationValid();

	};

}
