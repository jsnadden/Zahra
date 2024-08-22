#pragma once

#include "RenderCommand.h"
#include "Camera.h"
#include "Texture.h"
#include "EditorCamera.h"

#include "Zahra/Scene/Components.h"

namespace Zahra
{
	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		static float GetLineThickness();
		static void SetLineThickness(float thickness);

	private:
		static void SubmitBatch();
		static void NewBatch();

	public:
		///////////////////////////////////////////////////////////////////////////////////////////////////////
		// PRIMITIVES

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& colour, int EntityID = -1);
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D> texture, const glm::vec4& tint = { 1.0f, 1.0f, 1.0f, 1.0f }, float tiling = 1.0f, int EntityID = -1);
		static void DrawCircle(const glm::mat4& transform, const glm::vec4& colour, float thickness, float fade, int EntityID = -1);
		static void DrawLine(const glm::vec3& end0, const glm::vec3& end1, const glm::vec4& colour);
		static void DrawRect(const glm::mat4& transform, const glm::vec4& colour);

		///////////////////////////////////////////////////////////////////////////////////////////////////////
		// SPRITES

		static void DrawSprite(const glm::mat4& transform, SpriteComponent& sprite, int EntityID = -1);

		///////////////////////////////////////////////////////////////////////////////////////////////////////
		// STATS

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

