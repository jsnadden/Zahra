#pragma once

#include "RenderCommand.h"

#include "Camera.h"
#include "Texture.h"

namespace Zahra
{
	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void EndScene();
		static void Flush();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

	private:
		static void SubmitBatch();
		static void NewBatch();

	public:
		///////////////////////////////////////////////////////////////////////////////////////////////////////
		// Rendering primitives

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& colour);
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D> texture, const glm::vec4& tint = { 1.0f, 1.0f, 1.0f, 1.0f }, float tiling = 1.0f);

		static void DrawQuad(const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& colour);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& dimensions, const glm::vec4& colour);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& dimensions, const Ref<Texture2D> texture, const glm::vec4& tint = { 1.0f, 1.0f, 1.0f, 1.0f }, float tiling = 1.0f);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& dimensions, const Ref<Texture2D> texture, const glm::vec4& tint = { 1.0f, 1.0f, 1.0f, 1.0f }, float tiling = 1.0f);

		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& dimensions, float rotation, const glm::vec4& colour);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& dimensions, float rotation, const glm::vec4& colour);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& dimensions, float rotation, const Ref<Texture2D> texture, const glm::vec4& tint = { 1.0f, 1.0f, 1.0f, 1.0f }, float tiling = 1.0f);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& dimensions, float rotation, const Ref<Texture2D> texture, const glm::vec4& tint = { 1.0f, 1.0f, 1.0f, 1.0f }, float tiling = 1.0f);

		///////////////////////////////////////////////////////////////////////////////////////////////////////
		// Rendering stats

		struct Statistics
		{
			uint32_t DrawCalls;
			uint32_t QuadCount;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
		};

		static Statistics GetStats();
		static void ResetStats();
	};

};

