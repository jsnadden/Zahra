#pragma once

#include "Zahra/Renderer/Texture.h"

#include <glad/glad.h>

namespace Zahra
{

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::filesystem::path& path);
		OpenGLTexture2D(uint32_t width, uint32_t height);
		virtual ~OpenGLTexture2D() override;

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetRendererID() const override { return m_RendererID; }

		virtual const std::filesystem::path& GetFilepath() const override;

		virtual void SetData(void* data, uint32_t size) override;

		virtual void Bind(uint32_t slot = 0) const override;

		virtual bool operator==(const Texture& other) const override { return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID; }

	private:
		std::filesystem::path m_Filepath; // TODO: this really belongs in an asset manager class
		uint32_t m_RendererID;
		uint32_t m_Width, m_Height;
		GLenum m_InternalFormat, m_DataFormat;

	};

}

