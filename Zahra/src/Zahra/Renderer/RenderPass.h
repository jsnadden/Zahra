//#pragma once
//
//#include "Zahra/Core/Ref.h"
//
//namespace Zahra
//{
//	enum class AttachmentFormat
//	{
//		None = 0,
//
//		// Colour
//		RGBA8,
//		RGBA16F,
//		RED_INTEGER,
//
//		SRGBA8,
//
//		// Depth/Stencil
//		DEPTH24STENCIL8,
//
//		// Defaults
//		Depth = DEPTH24STENCIL8
//	};
//
//	struct RenderPassSpecification
//	{
//		AttachmentFormat attachmentFormat = AttachmentFormat::SRGBA8;
//	};
//
//	class RenderPass : public RefCounted
//	{
//	public:
//		virtual ~RenderPass() = default;
//
//		virtual RenderPassSpecification& GetSpecification() = 0;
//		virtual const RenderPassSpecification& GetSpecification() const = 0;
//
//		static Ref<RenderPass> Create(RenderPassSpecification specification);
//	};
//
//}
