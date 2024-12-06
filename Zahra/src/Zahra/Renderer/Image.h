#pragma once

namespace Zahra
{
	enum class ImageUsage
	{
		ColourAttachment,
		DepthStencilAttachment,
		Texture
	};

	enum class ImageFormat
	{
		Unspecified,

		// single channel
		R8UN,
		R8UI, R16UI, R32UI,
		R32F,

		// two channels
		RG8,
		RG16F, RG32F,

		// three channels
		RGB, SRGB,
		B10R11G11UF,

		// four channels
		RGBA, SRGBA,
		RGBA16F, RGBA32F
	};

	enum class ImageLayout
	{
		Undefined
	};

	class Image : public RefCounted
	{
	public:
		
		virtual ~Image() {}

		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;

		static Ref<Image> Create(uint32_t width, uint32_t height, ImageFormat format, ImageUsage usage);

	};

}
