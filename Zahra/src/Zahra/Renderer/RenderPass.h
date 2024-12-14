#pragma once

#include "Zahra/Renderer/Framebuffer.h"
#include "Zahra/Renderer/Image.h"
#include "Zahra/Renderer/RendererTypes.h"
#include "Zahra/Renderer/Shader.h"
#include "Zahra/Renderer/Texture.h"
#include "Zahra/Renderer/VertexBuffer.h"

namespace Zahra
{
	// TODO:
	// - multiple subpasses (and their associated dependencies)\
	// - framebuffer attachment aliasing (and thus multiple attachments targeting the swapchain)
	// - allow separate depth/stencil attachments
	// - for that matter, we're not using the stencil value at all
	// - support for instanced rendering
	// - multisampling
	// - mipmapping
	// - blending parameters in specification
	// - push constants
	// - resizing
	// - output to texture

	struct RenderPassSpecification
	{
		Ref<Shader> Shader;
		PrimitiveTopology Topology = PrimitiveTopology::Triangles;

		FramebufferSpecification FramebufferSpec;
		
		bool BackfaceCulling = true;
	};

	class RenderPass : public RefCounted
	{
	public:
		virtual ~RenderPass() = default;

		virtual const RenderPassSpecification& GetSpecification() const = 0;
		virtual const Ref<Framebuffer> GetFramebuffer() const = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		static Ref<RenderPass> Create(const RenderPassSpecification& specification);
	};

}
