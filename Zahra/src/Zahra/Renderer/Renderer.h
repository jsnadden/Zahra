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
	// NOTE: this is currently an experimental mess, and plenty of
	// it will be migrated elsewhere or deleted entirely
	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static RendererConfig& GetConfig();
		static void SetConfig(const RendererConfig& config);

		static void BeginFrame();
		static void EndFrame();

		// TODO: split up a bunch of this functionality into SceneRenderer and 2DRenderer classes
		static void BeginScene(const glm::mat4& view, const glm::mat4& projection);
		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();

		static void Present();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static uint32_t GetSwapchainWidth();
		static uint32_t GetSwapchainHeight();

		static uint32_t GetCurrentFrameIndex();
		static uint32_t GetFramesInFlight();

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		static Ref<RendererContext> GetContext() { return Application::Get().GetWindow().GetRendererContext(); }

		static void DrawTestScene();
		static void ClearPass();

		static const Ref<Image2D>& GetRenderTarget();

		///////////////////////////////////////////////////////////////////////////////////////////////////////
		// PRIMITIVES

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& colour, int entityID = -1);
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D> texture, const glm::vec4& tint = { 1.0f, 1.0f, 1.0f, 1.0f }, float tiling = 1.0f, int entityID = -1);
		static void DrawCircle(const glm::mat4& transform, const glm::vec4& colour, float thickness, float fade, int entityID = -1);
		static void DrawLine(const glm::vec3& end0, const glm::vec3& end1, const glm::vec4& colour, int entityID = -1);
		static void DrawRect(const glm::mat4& transform, const glm::vec4& colour, int entityID = -1);

		// TODO: particles?

		///////////////////////////////////////////////////////////////////////////////////////////////////////
		// SPRITES

		// TODO: move to SceneRenderer
		static void DrawSprite(const glm::mat4& transform, SpriteComponent& sprite, int entityID = -1);

		///////////////////////////////////////////////////////////////////////////////////////////////////////
		// STATS

		struct Statistics
		{
			uint32_t QuadBatchCount = 0;
			uint32_t QuadCount = 0;

			uint32_t CircleBatchCount = 0;
			uint32_t CircleCount = 0;

			uint32_t LineBatchCount = 0;
			uint32_t LineCount = 0;

			//uint32_t TotalAllocatedMemory = 0;

			uint32_t DrawCalls = 0;

			/*uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }*/
		};

		static const Statistics& GetStats();
		static void ResetStats();

	private:
		static void AddNewQuadBatch();

		// TODO: similar methods for circle/line batches
	};

};

