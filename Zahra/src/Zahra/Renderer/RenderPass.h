#pragma once

#include "Zahra/Renderer/Image.h"
#include "Zahra/Renderer/RendererTypes.h"
#include "Zahra/Renderer/Shader.h"
#include "Zahra/Renderer/Texture.h"
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

	struct AttachmentSpecification
	{
		ImageFormat Format; // ignored if attachment is using a swapchain image
		AttachmentLoadOp LoadOp;
		AttachmentStoreOp StoreOp;

		// TODO: not currently used
		/*ImageLayout InitialLayout;
		ImageLayout FinalLayout;*/
	};

	struct RenderPassSpecification
	{
		Ref<Shader> Shader;

		PrimitiveTopology Topology = PrimitiveTopology::Triangles;

		bool TargetSwapchain = true;
		bool HasDepthStencil = true;
		uint32_t AttachmentWidth, AttachmentHeight; // ignored if attachment is using a swapchain image
		AttachmentSpecification PrimaryAttachment; // ignored if attachment is using a swapchain image
		std::vector<AttachmentSpecification> AdditionalAttachments;

		bool BackfaceCulling = true;
	};

	class RenderPass : public RefCounted
	{
	public:
		virtual ~RenderPass() = default;

		virtual RenderPassSpecification& GetSpecification() = 0;
		virtual const RenderPassSpecification& GetSpecification() const = 0;

		virtual Ref<Texture2D> TextureFromPrimaryAttachment() const = 0;

		virtual void Refresh() = 0;

		static Ref<RenderPass> Create(const RenderPassSpecification& specification);
	};

}
