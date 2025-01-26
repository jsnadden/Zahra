#pragma once

#include "Zahra/Renderer/Image.h"

#include <variant>

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

		union
		{
			glm::vec4 fColour = glm::vec4(.0f, .0f, .0f, 1.0f);
			glm::ivec4 iColour;
		} ClearColour;
		
		//AttachmentLoadOp LoadOp = AttachmentLoadOp::Unspecified;
		//AttachmentStoreOp StoreOp = AttachmentStoreOp::Unspecified;

		// add blending options
	};
	
	struct FramebufferSpecification
	{
		std::string Name; // for debugging

		uint32_t Width = 0, Height = 0;

		// TODO: implement MSAA (and other AA methods) once I have a clearer
		// idea of the render target workflow in SceneRenderer
		//uint32_t Multisampling = 1;
		
		std::vector<AttachmentSpecification> ColourAttachmentSpecs;

		bool HasDepthStencil = false;
		float DepthClearValue = 1.0f;
		AttachmentSpecification DepthStencilAttachmentSpec;
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
