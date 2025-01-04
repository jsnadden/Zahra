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
	// - multiple subpasses (and their associated dependencies)
	// - framebuffer attachment aliasing
	// - allow separate depth/stencil attachments
	// - support for instanced rendering
	// - multisampling
	// - mipmapping
	// - blending parameters in specification
	// - push constants

	struct RenderPassSpecification
	{
		std::string Name;

		// pipeline
		Ref<Shader> Shader;
		PrimitiveTopology Topology = PrimitiveTopology::Triangles;
		bool BackfaceCulling = false;
		bool DynamicLineWidths = false;

		// render target
		Ref<Framebuffer> RenderTarget; // if not set, will target swapchain instead
		bool ClearColourAttachments = false;
		bool ClearDepthAttachment = false;
	};

	class RenderPass : public RefCounted
	{
	public:
		virtual ~RenderPass() = default;

		virtual const RenderPassSpecification& GetSpecification() const = 0;
		virtual const Ref<Framebuffer> GetRenderTarget() const = 0;

		virtual bool TargetSwapchain() = 0;

		// to be called AFTER swapchain or target framebuffer has already been resized
		virtual void OnResize() = 0;

		static Ref<RenderPass> Create(const RenderPassSpecification& specification);
	};

}
