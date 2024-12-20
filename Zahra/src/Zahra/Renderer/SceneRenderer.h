#pragma once

#include "Zahra/Renderer/RenderCommandQueue.h"
#include "Zahra/Renderer/Shader.h"

namespace Zahra
{

	struct SceneRendererSpecification
	{
		int Placeholder = 0;
	};

	class SceneRenderer : public RefCounted
	{
	public:
		SceneRenderer(const SceneRendererSpecification& specification);
		~SceneRenderer();

	private:
	};

}

