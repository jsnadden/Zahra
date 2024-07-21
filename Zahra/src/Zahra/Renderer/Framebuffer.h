#pragma once


namespace Zahra
{
	enum class FramebufferTextureFormat
	{
		None = 0,

		// Colour
		RGBA8,
		RGBA16F,
		RED_INTEGER,

		// Depth+Stencil
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8
	};

	namespace Utils
	{
		static bool IsDepthFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:
			{
				return true;
			}
			}

			return false;
		}
	}

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format)
			:TextureFormat(format) {}

		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
		// TODO: specify a "clear" value (not sure how to do this without specifying a type... is void* safe? can this be templated?)
		// TODO: filtering and wrapping
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> specs)
		: TextureSpecs(specs) {}

		std::vector<FramebufferTextureSpecification> TextureSpecs;
	};

	struct FramebufferSpecification
	{
		FramebufferAttachmentSpecification AttachmentSpec;
		uint32_t Width, Height;
		uint32_t Samples = 1;

		//FrameBufferFormat Format = 

		// SwapChainTarget determines whether we render directly to the screen, or just store the result (necessary for Vulkan rendering)
		bool SwapChainTarget = false;
	};

	class Framebuffer
	{
	public:

		virtual ~Framebuffer() = default;
		
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetColourAttachmentID(int index = 0) const = 0;

		virtual void ClearColourAttachment(int attachmentIndex, int clearValue) = 0;

		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		// TODO: might implement the non-const getter later, but it would require some infrastructure
		// virtual FramebufferSpecification& GetSpecification() = 0;
		virtual const FramebufferSpecification& GetSpecification() const = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

	};

}


