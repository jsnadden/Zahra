#pragma once

#include "RendererAPI.h"

namespace Zahra
{
	
	class RenderCommand
	{
	public:
		inline static void Init()
		{
			s_rendererAPI->Init();
		}

		inline static void SetClearColour(const glm::vec4& colour)
		{
			s_rendererAPI->SetClearColour(colour);
		}

		inline static void Clear()
		{
			s_rendererAPI->Clear();
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray)
		{
			s_rendererAPI->DrawIndexed(vertexArray);
		}


	private:

		static RendererAPI* s_rendererAPI;

	};

}