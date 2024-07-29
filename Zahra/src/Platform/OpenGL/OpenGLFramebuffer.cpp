#include "zpch.h"
#include "OpenGLFramebuffer.h"

#include <glad/glad.h>

namespace Zahra
{

	static const uint32_t s_MaxFramebufferDimension = 8192; // TODO: this should not be hardcoded - read it in from GPU capabilities!

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& fbSpec)
		: m_Specification(fbSpec)
	{
		for (auto spec : m_Specification.AttachmentSpec.TextureSpecs)
		{
			if (Utils::IsDepthFormat(spec.TextureFormat))
				m_DepthAttachmentSpec = spec;
			else
				m_ColourAttachmentSpecs.emplace_back(spec);
		}

		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		ClearFramebuffer();
	}

	void OpenGLFramebuffer::Invalidate()
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// REFRESH
		if (m_RendererID)
		{
			ClearFramebuffer();
			m_ColourAttachmentIDs.clear();
			m_DepthAttachmentID = 0;
		}
		
		//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// GENERATE NEW FRAMEBUFFER
		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		//
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// COLOUR ATTACHMENTS
		bool multisampled = (m_Specification.Samples > 1);

		if (m_ColourAttachmentSpecs.size())
		{
			m_ColourAttachmentIDs.resize(m_ColourAttachmentSpecs.size());
			CreateTextures(multisampled, m_ColourAttachmentIDs.data(), (uint32_t)m_ColourAttachmentIDs.size());
			
			for (size_t i = 0; i < m_ColourAttachmentIDs.size(); i++)
			{
				BindTexture(multisampled, m_ColourAttachmentIDs[i]);

				switch (m_ColourAttachmentSpecs[i].TextureFormat)
				{
					case FramebufferTextureFormat::RGBA8:
					{
						AttachColourTexture(m_ColourAttachmentIDs[i], m_Specification.Samples, GL_RGBA8, GL_RGBA, m_Specification.Width, m_Specification.Height, i);
						break;
					}

					case FramebufferTextureFormat::RED_INTEGER:
					{
						AttachColourTexture(m_ColourAttachmentIDs[i], m_Specification.Samples, GL_R32I, GL_RED_INTEGER, m_Specification.Width, m_Specification.Height, i);
						break;
					}
				}

			}
		}
		//
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// DEPTH ATTACHMENT
		if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
		{
			CreateTextures(multisampled, &m_DepthAttachmentID, 1);
			BindTexture(multisampled, m_DepthAttachmentID);

			switch (m_DepthAttachmentSpec.TextureFormat)
			{
				case FramebufferTextureFormat::DEPTH24STENCIL8:
				{
					AttachDepthTexture(m_DepthAttachmentID, m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
					break;
				}
			}
		}
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////

		if (m_ColourAttachmentIDs.size() > 1)
		{
			Z_CORE_ASSERT(m_ColourAttachmentIDs.size() <= 4, "Currently only supports up to 4 colour attachments");

			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers((GLsizei)m_ColourAttachmentIDs.size(), buffers);
		}
		else if (m_ColourAttachmentIDs.empty())
		{
			// only do a depth pass, for shadow mapping etc.
			glDrawBuffer(GL_NONE);
		}

		Z_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::ClearFramebuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures((GLsizei)m_ColourAttachmentIDs.size(), m_ColourAttachmentIDs.data());
		glDeleteTextures(1, &m_DepthAttachmentID);
	}

	void OpenGLFramebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
		m_bound = true;
	}

	void OpenGLFramebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_bound = false;
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > s_MaxFramebufferDimension || height > s_MaxFramebufferDimension)
		{
			Z_CORE_WARN("Attempted to resize Framebuffer to invalid value: {0}x{1}", width, height);
			return;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;
		Invalidate();
	}

	uint32_t OpenGLFramebuffer::GetColourAttachmentID(int index) const
	{
		Z_CORE_ASSERT(index < m_ColourAttachmentIDs.size(), "Invalid colour attachment index");
		return m_ColourAttachmentIDs[index];
	}

	void OpenGLFramebuffer::ClearColourAttachment(int attachmentIndex, int clearValue)
	{
		Z_CORE_ASSERT(attachmentIndex < m_ColourAttachmentIDs.size(), "Invalid colour attachment index");
		
		glClearTexImage(m_ColourAttachmentIDs[attachmentIndex], 0, GL_RED_INTEGER, GL_INT, &clearValue);
	}

	int OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		Z_CORE_ASSERT(m_bound, "Framebuffer is not currently bound");
		Z_CORE_ASSERT(attachmentIndex < m_ColourAttachmentIDs.size(), "Invalid colour attachment index");

		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		int pixelData;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData); // TODO: don't hardcode parameters
		return pixelData;
	}



	uint32_t OpenGLFramebuffer::TextureTarget(bool multisampled)
	{
		return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
	}

	void OpenGLFramebuffer::CreateTextures(bool multisampled, uint32_t* outID, uint32_t count) // populates array at outID with texture IDs
	{
		glCreateTextures(TextureTarget(multisampled), count, outID);
	}

	void OpenGLFramebuffer::BindTexture(bool multisampled, uint32_t id)
	{
		glBindTexture(TextureTarget(multisampled), id);
	}

	void OpenGLFramebuffer::AttachColourTexture(uint32_t id, int samples, uint32_t internalFormat, uint32_t format, uint32_t width, uint32_t height, size_t index)
	{
		bool multisampled = (samples > 1);
		
		if (multisampled)
		{
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
			// TODO: why don't we set parameters here, like in the single sample case below?
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

			// TODO: make these part of our texturespec struct
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (GLenum)index, TextureTarget(multisampled), id, 0);

	}

	void OpenGLFramebuffer::AttachDepthTexture(uint32_t id, int samples, uint32_t format, uint32_t type, uint32_t width, uint32_t height)
	{
		bool multisampled = (samples > 1);

		if (multisampled)
		{
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
		}
		else
		{
			glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, type, TextureTarget(multisampled), id, 0);

	}





}

