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
		void ClearFramebuffer();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual uint32_t GetColourAttachmentRendererID() const override { return m_ColourAttachment; }

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

	private:
		FramebufferSpecification m_Specification;
		uint32_t m_RendererID = 0;
		uint32_t m_ColourAttachment = 0, m_DepthAttachment = 0;
	};
}

