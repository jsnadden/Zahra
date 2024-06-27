#pragma once

#include "Camera.h"
#include "Texture.h"

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
		static void DrawQuad(const glm::vec3& position, const glm::vec2& dimensions, const Ref<Texture> texture, const glm::vec4& colour = { 1.0f, 1.0f, 1.0f, 1.0f }, float rotation = .0f);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& dimensions, const Ref<Texture> texture, const glm::vec4& colour = { 1.0f, 1.0f, 1.0f, 1.0f }, float rotation = .0f);
		
		static void DrawQuad(const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& colour, float rotation = .0f);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& dimensions, const glm::vec4& colour, float rotation = .0f);
	};

}


