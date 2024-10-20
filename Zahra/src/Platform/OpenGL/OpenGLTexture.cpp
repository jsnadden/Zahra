#include "zpch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"

namespace Zahra
{

	OpenGLTexture2D::OpenGLTexture2D(const std::filesystem::path& path)
		: m_Filepath(path)
	{
		int width, height, channels;

		stbi_set_flip_vertically_on_load(true);
		stbi_uc* data = nullptr;
		data = stbi_load(path.string().c_str(), &width, &height, &channels, 0);
		
		Z_CORE_ASSERT(data, "Failed to load image.");

		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;

		switch (channels)
		{
		case 3:
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
			break;
		}
		case 4:
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
			break;
		}
		}

		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		Z_CORE_ASSERT(internalFormat & dataFormat, "Format not supported.");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

		//TODO: make these options configurable
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height)
	{
		Z_PROFILE_FUNCTION();

		//TODO: make this stuff configurable

		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	const std::filesystem::path& OpenGLTexture2D::GetFilepath() const
	{
		return m_Filepath;
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		Z_PROFILE_FUNCTION();

		uint32_t bpp = (m_DataFormat == GL_RGBA ? 4 : 3); // TODO: generalise this if we use formats other than rgb/rgba
		Z_CORE_ASSERT(size == m_Width * m_Height * bpp, "Incomplete texture data.")

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		Z_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_RendererID);
	}

}
