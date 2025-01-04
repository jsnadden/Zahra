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
		R8_UN, R8_UI, R16_UI, R32_UI, R32_SI,
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

	struct Image2DSpecification
	{
		std::string Name = "anonymous_image"; // for debugging
		uint32_t Width = 1, Height = 1;
		ImageFormat Format = ImageFormat::Unspecified;
		ImageLayout InitialLayout = ImageLayout::Unspecified;
		bool Sampled = false;
		bool TransferSource = false;
		bool TransferDestination = false;
		bool CreatePixelBuffer = false;
	};

	class Image : public RefCounted
	{
	public:
		virtual ~Image() = default;

		static uint32_t BytesPerPixel(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::R32_SI:	return 4;				
				case ImageFormat::RGBA_UN:	return 4;
				case ImageFormat::SRGBA:	return 4;
			}

			Z_CORE_ASSERT(false, "Unsupported colour format");
			return 0;
		}
	};

	class Image2D : public Image
	{
	public:		
		virtual const Image2DSpecification GetSpecification() const = 0;
		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual void* ReadPixel(int32_t x, int32_t y) = 0;

		static Ref<Image2D> Create(Image2DSpecification specification);

	};

}
