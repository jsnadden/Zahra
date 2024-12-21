#pragma once

namespace Zahra
{
	enum class ImageUsage
	{
		Unspecified,
		FramebufferAttachment,
		Texture
	};

	enum class ImageFormat
	{
		Unspecified,

		// single channel
		R8_UN, R8_UI, R16_UI, R32_UI,
		R32_F,

		// two channels
		RG8_UN,
		RG16_F, RG32_F,

		// three channels
		RGB, SRGB,
		B10R11G11_UF,

		// four channels
		RGBA_UN, SRGBA,
		RGBA16_F, RGBA32_F,

		// depth stencil
		DepthStencil
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

	struct Image2DSpecification
	{
		std::string Name; // for debugging
		uint32_t Width, Height;
		ImageFormat Format = ImageFormat::Unspecified;
		bool Sampled;
		bool TransferSource;
		bool TransferDestination;
	};

	class Image2D : public RefCounted
	{
	public:
		
		virtual ~Image2D() {}

		virtual const Image2DSpecification GetSpecification() const = 0;
		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		static Ref<Image2D> Create(Image2DSpecification specification);

	};

}
