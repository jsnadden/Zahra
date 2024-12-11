#pragma once

#include "Zahra/Renderer/Image.h"

namespace Zahra
{
	enum class AttachmentLoadOp
	{
		Load, Clear, Unspecified
	};

	enum class AttachmentStoreOp
	{
		Store, Unspecified
	};

	struct AttachmentSpecification
	{
		Ref<Image> InheritFrom;

		ImageFormat Format;

		glm::vec3 ClearColour;
		AttachmentLoadOp LoadOp;
		AttachmentStoreOp StoreOp;

		ImageLayout InitialLayout;
		ImageLayout FinalLayout;
	};
	
	struct FramebufferSpecification
	{
		bool TargetSwapchain;
		bool HasDepthStencil;

		uint32_t Width, Height; // ignored if attachment is using a swapchain image

		std::vector<AttachmentSpecification> ColourAttachments;
		AttachmentSpecification DepthStencilAttachment;
	};

	class Framebuffer : public RefCounted
	{
	public:

		virtual ~Framebuffer() = default;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		virtual const FramebufferSpecification& GetSpecification() const = 0;
		virtual const Ref<Image>& GetColourAttachment(uint32_t index) const = 0;
		virtual const Ref<Image>& GetDepthStencilAttachment() const = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

	};

}
