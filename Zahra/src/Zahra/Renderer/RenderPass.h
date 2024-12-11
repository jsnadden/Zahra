#pragma once

#include "Zahra/Renderer/Framebuffer.h"
#include "Zahra/Renderer/Image.h"
#include "Zahra/Renderer/RendererTypes.h"
#include "Zahra/Renderer/Shader.h"
#include "Zahra/Renderer/Texture.h"
#include "Zahra/Renderer/VertexBuffer.h"

namespace Zahra
{
	

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

		virtual const Ref<Framebuffer> GetColourAttachment(uint32_t index) const = 0;
		virtual const std::vector<Ref<Framebuffer>> GetColourAttachments() const = 0;
		virtual const Ref<Framebuffer> GetDepthStencilAttachment() const = 0;

		virtual void Refresh() = 0;

		static Ref<RenderPass> Create(const RenderPassSpecification& specification);
	};

}
