#pragma once

#include "Zahra/Renderer/Framebuffer.h"

namespace Zahra
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer();

		void Regenerate();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual uint32_t GetColourAttachmentRendererID() const override { return m_ColourAttachment; }

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

	private:
		uint32_t m_RendererID;
		FramebufferSpecification m_Specification;
		uint32_t m_ColourAttachment, m_DepthAttachment;
	};
}

