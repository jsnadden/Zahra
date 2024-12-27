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
		Ref<Image2D> InheritFrom;

		ImageFormat Format = ImageFormat::Unspecified;

		//glm::vec3 ClearColour;
		//AttachmentLoadOp LoadOp = AttachmentLoadOp::Unspecified;
		//AttachmentStoreOp StoreOp = AttachmentStoreOp::Unspecified;

		// add blending options
	};
	
	struct FramebufferSpecification
	{
		std::string Name; // for debugging

		uint32_t Width = 1.0f, Height = 1.0f;

		glm::vec3 ClearColour = { 0.f, 0.f, 0.f };
		std::vector<AttachmentSpecification> ColourAttachmentSpecs;

		bool HasDepthStencil = false;
		float DepthClearValue = 1.0f;
		AttachmentSpecification DepthStencilAttachmentSpec;

		// add multisampling options
	};

	class Framebuffer : public RefCounted
	{
	public:

		virtual ~Framebuffer() = default;

		//virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		virtual const FramebufferSpecification& GetSpecification() const = 0;

		virtual Ref<Image2D> GetColourAttachment(uint32_t index) const = 0;
		virtual Ref<Image2D> GetDepthStencilAttachment() const = 0;

		virtual uint32_t GetColourAttachmentCount() const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

	};

}
