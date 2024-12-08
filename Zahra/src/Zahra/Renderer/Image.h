#pragma once

namespace Zahra
{
	enum class ImageUsage
	{
		ColourAttachment,
		DepthStencilAttachment,
		Texture,
		RenderToTexture
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

	// TODO: make room for 3d images

	struct ImageSpecification
	{
		uint32_t Width, Height;
		ImageFormat Format;
		ImageUsage Usage;
		ImageLayout InitialLayout;
	};

	class Image : public RefCounted
	{
	public:
		
		virtual ~Image() {}

		virtual const ImageSpecification GetSpecification() const = 0;
		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;

		//virtual void CopyData(Ref<Image>& source) = 0; // TODO: generalise this method with src/dst subregions

		static Ref<Image> Create(ImageSpecification specification);

	};

}
