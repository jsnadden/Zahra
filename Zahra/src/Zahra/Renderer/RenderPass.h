#pragma once

#include "Zahra/Renderer/RendererTypes.h"
#include "Zahra/Renderer/Shader.h"
#include "Zahra/Renderer/VertexBuffer.h"

namespace Zahra
{
	enum class AttachmentLoadOp
	{
		Load, Clear, Unspecified
	};

	enum class AttachmentStoreOp
	{
		Store, Unspecified
	};

	enum class AttachmentLayout
	{
		Undefined = 0,
		Colour,
		DepthStencil,
		Present
	};

	struct RenderPassSpecification
	{
		Ref<Shader> Shader;
		VertexBufferLayout VertexLayout;
		PrimitiveTopology Topology = PrimitiveTopology::Triangles;

		// currently assuming only one colour attachment
		AttachmentLoadOp LoadOp;
		AttachmentStoreOp StoreOp;
		AttachmentLayout InitialLayout;
		AttachmentLayout FinalLayout;

		bool HasDepthStencil = true;
	};

	class RenderPass : public RefCounted
	{
	public:
		virtual ~RenderPass() = default;

		virtual RenderPassSpecification& GetSpecification() = 0;
		virtual const RenderPassSpecification& GetSpecification() const = 0;

		virtual void RefreshFramebuffers() = 0;

		static Ref<RenderPass> Create(const RenderPassSpecification& specification);
	};

}
