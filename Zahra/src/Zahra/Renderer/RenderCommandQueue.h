#pragma once

#include "RendererAPI.h"

namespace Zahra
{
	// TODO: for the time being this will simply forward everything as-is
	// to RendererAPI, but eventually this will ACTUALLY be a queue to be
	// processed during a later frame
	class RenderCommandQueue
	{
	public:
		//static void Init()
		//{
		//	s_RendererAPI->Init();
		//}

		//static void Shutdown()
		//{
		//	s_RendererAPI->Shutdown();
		//}

		//static void NewFrame()
		//{
		//	s_RendererAPI->BeginFrame();
		//}

		//static void EndRenderPass()
		//{
		//	//s_RendererAPI->EndRenderPass();
		//}

		//static void PresentImage()
		//{
		//	s_RendererAPI->Present();
		//}


		//static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		//{
		//	s_RendererAPI->SetViewport(x, y, width, height);
		//}

		///*static void Clear()
		//{
		//	s_RendererAPI->Clear();
		//}

		//static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0)
		//{
		//	s_RendererAPI->DrawIndexed(vertexArray, indexCount);
		//}

		//static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount = 0)
		//{
		//	s_RendererAPI->DrawLines(vertexArray, vertexCount);
		//}*/

	private:

		static Scope<RendererAPI> s_RendererAPI;

	};

}
