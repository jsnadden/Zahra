#pragma once

#include "Zahra/Core/Application.h"
#include "Zahra/Renderer/Camera.h"
#include "Zahra/Renderer/EditorCamera.h"
#include "Zahra/Renderer/RenderCommandQueue.h"
#include "Zahra/Renderer/RendererConfig.h"
#include "Zahra/Renderer/Texture.h"
#include "Zahra/Scene/Components.h"

namespace Zahra
{
	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static RendererConfig& GetConfig();
		static void SetConfig(const RendererConfig& config);

		static void BeginFrame();
		static void EndFrame();

		// TODO: move this stuff to SceneRenderer
		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();

		static void Present();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static uint32_t GetCurrentFrameIndex();
		static uint32_t GetFramesInFlight();

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		static Ref<RendererContext> GetContext() { return Application::Get().GetWindow().GetRendererContext(); }

		/*static float GetLineThickness();
		static void SetLineThickness(float thickness);*/

	private:
		static void SubmitBatch();
		static void NewBatch();

	public:

		// TEMPORARY
		static void DrawTutorialScene(Ref<Texture2D>& outputTexture);

		///////////////////////////////////////////////////////////////////////////////////////////////////////
		// PRIMITIVES

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& colour, int entityID = -1);
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D> texture, const glm::vec4& tint = { 1.0f, 1.0f, 1.0f, 1.0f }, float tiling = 1.0f, int entityID = -1);
		static void DrawCircle(const glm::mat4& transform, const glm::vec4& colour, float thickness, float fade, int entityID = -1);
		static void DrawLine(const glm::vec3& end0, const glm::vec3& end1, const glm::vec4& colour, int entityID = -1);
		static void DrawRect(const glm::mat4& transform, const glm::vec4& colour, int entityID = -1);

		///////////////////////////////////////////////////////////////////////////////////////////////////////
		// SPRITES

		static void DrawSprite(const glm::mat4& transform, SpriteComponent& sprite, int entityID = -1);

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

