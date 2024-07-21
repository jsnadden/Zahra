#pragma once

#include "Zahra/Renderer/Framebuffer.h"

namespace Zahra
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer();

		void Invalidate();
		void ClearFramebuffer();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual uint32_t GetColourAttachmentID(int index = 0) const override;

		virtual void ClearColourAttachment(int attachmentIndex, int clearValue) override;

		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

	private:
		FramebufferSpecification m_Specification;
		uint32_t m_RendererID = 0;

		bool m_bound = false;

		std::vector<FramebufferTextureSpecification> m_ColourAttachmentSpecs;
		FramebufferTextureSpecification m_DepthAttachmentSpec = { FramebufferTextureFormat::None }; // by default, no depth buffer

		std::vector<uint32_t> m_ColourAttachmentIDs;
		uint32_t m_DepthAttachmentID = 0;

		uint32_t TextureTarget(bool multisampled);
		void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count);
		void BindTexture(bool multisampled, uint32_t id);
		void AttachColourTexture(uint32_t id, int samples, uint32_t internalFormat, uint32_t format, uint32_t width, uint32_t height, size_t index);
		void AttachDepthTexture(uint32_t id, int samples, uint32_t format, uint32_t type, uint32_t width, uint32_t height);
	};
}

