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

		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual const FramebufferSpecification& GetSpecification() const { return m_Specification; }
		virtual const Ref<Image>& GetColourAttachment(uint32_t index) const { return m_ColourAttachments[index]; }
		virtual const Ref<Image>& GetDepthStencilAttachment() const { return m_DepthsStencilAttachment; }

		const VkFramebuffer& GetVkFramebuffer() const { return m_VkFramebuffer; }

	private:
		FramebufferSpecification m_Specification;

		std::vector<Ref<VulkanImage>> m_ColourAttachments;
		Ref<VulkanImage> m_DepthsStencilAttachment;

		VkFramebuffer m_VkFramebuffer;
		
		void CreateAttachmentFromSwapchain();
		void CreateAttachmentFromSpecification(uint32_t index);
		void CreateDepthStencilAttachment();

		void CreateVkFramebuffer();

	};

}
