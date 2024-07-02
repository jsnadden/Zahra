#pragma once


namespace Zahra
{
	struct FramebufferSpecification
	{
		uint32_t Width, Height;

		uint32_t Samples = 1;

		//FrameBufferFormat Format = 

		// SwapChainTarget determines whether we render directly to the screen, or just store the result (necessary for Vulkan rendering)
		bool SwapChainTarget = false;
	};

	class Framebuffer
	{
	public:
		
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual uint32_t GetColourAttachmentRendererID() const = 0;

		// TODO: might implement the non-const getter later, but it would require some infrastructure
		// virtual FramebufferSpecification& GetSpecification() = 0;
		virtual const FramebufferSpecification& GetSpecification() const = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);


	};

}


