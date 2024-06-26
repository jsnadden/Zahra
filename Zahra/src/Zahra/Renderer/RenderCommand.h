#pragma once

#include "RendererAPI.h"

namespace Zahra
{
	
	class RenderCommand
	{
	public:
		inline static void Init()
		{
			s_RendererAPI->Init();
		}

		inline static void SetClearColour(const glm::vec4& colour)
		{
			s_RendererAPI->SetClearColour(colour);
		}

		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, count);
		}

		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}


	private:

		static Scope<RendererAPI> s_RendererAPI;

	};

}