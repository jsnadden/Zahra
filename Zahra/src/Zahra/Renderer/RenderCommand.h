#pragma once

#include "RendererAPI.h"

namespace Zahra
{
	
	class RenderCommand
	{
	public:
		static void Init()
		{
			s_RendererAPI->Init();
		}

		static void SetClearColour(const glm::vec4& colour)
		{
			s_RendererAPI->SetClearColour(colour);
		}

		static void Clear()
		{
			s_RendererAPI->Clear();
		}

		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount);
		}

		static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount = 0)
		{
			s_RendererAPI->DrawLines(vertexArray, vertexCount);
		}

		static void SetLineThickness(float thickness)
		{
			s_RendererAPI->SetLineThickness(thickness);
		}

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}


	private:

		static Scope<RendererAPI> s_RendererAPI;

	};

}
