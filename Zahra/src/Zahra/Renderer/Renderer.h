#pragma once

#include "Zahra/Core/Application.h"
#include "Zahra/Renderer/Camera.h"
#include "Zahra/Renderer/EditorCamera.h"
#include "Zahra/Renderer/RendererAPI.h"
#include "Zahra/Renderer/RendererConfig.h"
#include "Zahra/Renderer/Texture.h"

namespace Zahra
{
	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static const RendererConfig& GetConfig();
		static void SetConfig(const RendererConfig& config);

		static uint32_t GetSwapchainWidth();
		static uint32_t GetSwapchainHeight();
		static uint32_t GetFramesInFlight();
		static uint32_t GetCurrentFrameIndex();

		static const Ref<Image2D>& GetPrimaryRenderTarget();
		static const Ref<Framebuffer>& GetPrimaryFramebuffer();

		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
		static Ref<RendererContext> GetContext() { return Application::Get().GetWindow().GetRendererContext(); }

		static void BeginFrame();
		static void EndFrame();

		static void BeginRenderPass(Ref<RenderPass>& renderPass, bool bindPipeline = true, bool clearAttachments = false);
		static void EndRenderPass();
		static void Present();

		static void OnWindowResize(uint32_t width, uint32_t height);

		static void Draw(Ref<RenderPass>& renderPass, Ref<ShaderResourceManager>& resourceManager, Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount);
		static void DrawIndexed(Ref<RenderPass>& renderPass, Ref<ShaderResourceManager>& resourceManager, Ref<VertexBuffer>& vertexBuffer, Ref<IndexBuffer>& indexBuffer, uint32_t indexCount = 0, uint32_t startingIndex = 0);
		static void DrawMesh(Ref<RenderPass>& renderPass, Ref<ShaderResourceManager>& resourceManager, Ref<StaticMesh>& mesh);
		
		static void DrawTestScene(glm::mat4 view, glm::mat4 projection);

		static void SetLineWidth(float width);

		struct Capabilities
		{
			uint32_t MaxBoundTextures;
		};
		static Capabilities& GetCapabilities();

		struct Statistics
		{
			uint32_t DrawCallCount;
		};
		static const Statistics& GetStats();

	};
};

