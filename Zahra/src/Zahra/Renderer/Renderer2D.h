#pragma once

#include "Camera.h"

namespace Zahra
{
	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		// Rendering primitives
		static void DrawQuad(const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& colour);


	};

}


