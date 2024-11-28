#pragma once

#include "Zahra/Renderer/RendererTypes.h"
#include "Zahra/Renderer/Shader.h"
#include "Zahra/Renderer/VertexBuffer.h"

namespace Zahra
{
	struct PipelineSpecification
	{
		Ref<Shader> Shader;
		VertexBufferLayout VertexLayout;
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
