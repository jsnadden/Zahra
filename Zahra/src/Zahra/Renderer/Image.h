#pragma once

namespace Zahra
{
	enum class ImageUsage
	{
		Unspecified,
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
		Unspecified,
		ColourAttachment,
		DepthStencilAttachment,
		Texture,
		TransferSource,
		TransferDestination,
		Presentation,
	};

	// TODO: make room for 3d images

	struct ImageSpecification
	{
		uint32_t Width, Height;
		ImageFormat Format = ImageFormat::Unspecified;
		ImageUsage Usage = ImageUsage::Unspecified;
		ImageLayout Layout = ImageLayout::Unspecified;
	};

	class Image : public RefCounted
	{
	public:
		
		virtual ~Image() {}

		virtual const ImageSpecification GetSpecification() const = 0;
		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;

		static Ref<Image> Create(ImageSpecification specification);

	};

}
