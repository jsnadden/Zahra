#pragma once

#include "Zahra/Renderer/CommandBuffer.h"
#include "Zahra/Renderer/Pipeline.h"
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

		// TEMPORARY
		void DrawStuff();

	private:
		Ref<Shader> m_Shader;
		Ref<Pipeline> m_Pipeline;
		Ref<CommandBuffer> m_CommandBuffer;
	};

}

