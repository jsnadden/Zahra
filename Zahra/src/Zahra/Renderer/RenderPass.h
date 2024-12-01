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

	enum class AttachmentFormat
	{
		R8UN,
		R8UI,
		R16UI,
		R32UI,
		R32F,

		RG8,
		RG16F,
		RG32F,

		RGB,
		SRGB,

		RGBA,
		SRGBA,
		RGBA16F,
		RGBA32F,

		B10R11G11UF
	};

	struct AttachmentSpecification
	{
		AttachmentFormat Format; // ignored if attachment is using a swapchain image
		AttachmentLoadOp LoadOp;
		AttachmentStoreOp StoreOp;
		AttachmentLayout InitialLayout;
		AttachmentLayout FinalLayout;
	};

	struct RenderPassSpecification
	{
		Ref<Shader> Shader;

		VertexBufferLayout VertexLayout;

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

		virtual void Refresh() = 0;

		static Ref<RenderPass> Create(const RenderPassSpecification& specification);
	};

}
