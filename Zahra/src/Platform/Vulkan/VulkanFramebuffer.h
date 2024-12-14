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

		virtual const Ref<Image>& GetColourAttachment(uint32_t index) const;
		virtual const Ref<Image>& GetDepthStencilAttachment() const;

		const std::vector<VkClearValue> const GetClearValues();

		std::vector<VkFramebuffer> GenerateVkFramebuffers(const VkRenderPass& renderPass);

	private:
		FramebufferSpecification m_Specification;

		// Each attachment, except possibly m_ColourAttachments[0], consists of one image per frame-in-flight.
		// If targeting swapchain, m_ColourAttachments[0] will remain an empty vector
		std::vector<std::vector<Ref<VulkanImage>>> m_ColourAttachments;
		std::vector<VkImageView> m_SwapchainImageViews; // only populated if targeting swapchain
		std::vector<Ref<VulkanImage>> m_DepthStencilAttachment;
		
		void CreateAttachments();
		void DestroyAttachments();

		void CreateColourAttachmentFromSpecification(uint32_t index);
		void CreateDepthStencilAttachment();

	};

}
