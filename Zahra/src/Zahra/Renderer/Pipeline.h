#pragma once

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
	};

	class Pipeline : public RefCounted
	{

	};
}
