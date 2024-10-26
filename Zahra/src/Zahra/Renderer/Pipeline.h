#pragma once

#include "Zahra/Renderer/VertexBuffer.h"
#include "Zahra/Renderer/Framebuffer.h"
#include "Zahra/Renderer/RenderPass.h"
#include "Zahra/Renderer/Shader.h"

namespace Zahra
{
	enum class PrimitiveTopology
	{
		Triangles,
		TriangleStrip,
		TriangleFan,
		Lines,
		LineStrip,
		Points
	};

	struct PipelineSpecification
	{
		Ref<Shader> Shader;
		VertexBufferLayout VertexBufferLayout;
		PrimitiveTopology Topology = PrimitiveTopology::Triangles;
	};

	class Pipeline : public RefCounted
	{
	public:
		virtual ~Pipeline() = default;

		virtual PipelineSpecification& GetSpecification() = 0;
		virtual const PipelineSpecification& GetSpecification() const = 0;

		virtual void Invalidate() = 0;

		virtual Ref<Shader> GetShader() const = 0;

		static Ref<Pipeline> Create(const PipelineSpecification& specification);
	};
}
