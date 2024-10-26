#pragma once

#include "Zahra/Renderer/Pipeline.h"

#include <glm/glm.hpp>

namespace Zahra
{

	class RendererAPI
	{
	public:
		virtual ~RendererAPI() = default;

		enum class API
		{
			None = 0, OpenGL = 1, DX12 = 2, Vulkan = 3
		};

		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void OnWindowResize() = 0;

		virtual void SetClearColour(const glm::vec4& colour) = 0;

		virtual void NewFrame() = 0;

		virtual void BeginRenderPass(Ref<Pipeline> pipeline) = 0;
		virtual void EndRenderPass() = 0;

		virtual void PresentImage() = 0;

		inline static API GetAPI() { return s_API; }
		static RendererAPI* Create();
		
		// TEMPORARY
		virtual void TutorialDrawCalls() = 0;


	private:
		static API s_API;


	};

}
