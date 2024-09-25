#pragma once

#include "Zahra/Renderer/Shader.h"
#include "Zahra/Renderer/Buffer.h"
#include "Zahra/Renderer/Framebuffer.h"

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
		Ref<Framebuffer> TargetFramebuffer;
		BufferLayout VertexBufferLayout;
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
