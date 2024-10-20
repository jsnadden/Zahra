#pragma once

#include "Zahra/Renderer/VertexArray.h"

#include <glm/glm.hpp>

namespace Zahra
{

	class RendererAPI
	{
	public:
		virtual ~RendererAPI() = default;

		enum class API
		{
			None = 0, OpenGL = 1, Direct3D = 2, Vulkan = 3
		};

		virtual void Init() = 0;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		virtual void SetClearColour(const glm::vec4& colour) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount = 0) = 0;

		virtual void SetLineThickness(float thickness) = 0;

		inline static API GetAPI() { return s_API; }

	private:
		static API s_API;


	};

}
