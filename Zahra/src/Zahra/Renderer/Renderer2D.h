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
		static void Flush();

		///////////////////////////////////////////////////////////////////////////////////////////////////////
		// Rendering primitives

		static void DrawQuad(const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& colour);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& dimensions, const glm::vec4& colour);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& dimensions, const Ref<Texture2D> texture, const glm::vec4& tint = { 1.0f, 1.0f, 1.0f, 1.0f }, float tiling = 1.0f);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& dimensions, const Ref<Texture2D> texture, const glm::vec4& tint = { 1.0f, 1.0f, 1.0f, 1.0f }, float tiling = 1.0f);

		//static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& dimensions, float rotation, const Ref<Texture> texture, const glm::vec4& tint = { 1.0f, 1.0f, 1.0f, 1.0f }, float tiling = 1.0f);
		//static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& dimensions, float rotation, const Ref<Texture> texture, const glm::vec4& tint = { 1.0f, 1.0f, 1.0f, 1.0f }, float tiling = 1.0f);
		//static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& dimensions, float rotation, const glm::vec4& colour);
		//static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& dimensions, float rotation, const glm::vec4& colour);
	};

}


