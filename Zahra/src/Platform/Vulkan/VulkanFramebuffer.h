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

		// TODO: either need host-visible, memory-mapped images (frame perfect, but not great for general
		// performance), or to send mouse position to the fragment shader as a push constant, and output the
		// picked value to a host-visible storage buffer (which is therefore potentially several frames behind).
		// In the latter instance, this method would really belong to Renderer
		//virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual const FramebufferSpecification& GetSpecification() const { return m_Specification; }

		virtual Ref<Image2D> GetColourAttachment(uint32_t index) const;
		virtual Ref<Image2D> GetDepthStencilAttachment() const;

		std::vector<VkImageView> const GetImageViews();
		std::vector<VkClearValue> const GetClearValues();
		const std::vector<VkAttachmentDescription>& GetAttachmentDescriptions() const { return m_AttachmentDescriptions; }

		virtual void Resize(uint32_t width, uint32_t height) override;

	private:
		FramebufferSpecification m_Specification;

		std::vector<Ref<VulkanImage2D>> m_ColourAttachments;
		Ref<VulkanImage2D> m_DepthStencilAttachment;

		std::vector<VkAttachmentDescription> m_AttachmentDescriptions;
		
		void CreateAttachments();
		void DestroyAttachments();

		void CreateColourAttachment(uint32_t index);
		void CreateDepthStencilAttachment();

		bool ValidateSpecification();

	};

}
