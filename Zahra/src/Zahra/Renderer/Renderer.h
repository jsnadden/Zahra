#pragma once

#include "Zahra/Core/Application.h"
#include "Zahra/Renderer/Cameras/Camera.h"
#include "Zahra/Renderer/Cameras/EditorCamera.h"
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

		static void Draw(Ref<RenderPass>& renderPass, Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount);
		static void DrawIndexed(Ref<RenderPass>& renderPass, Ref<VertexBuffer>& vertexBuffer, Ref<IndexBuffer>& indexBuffer, uint32_t indexCount = 0, uint32_t startingIndex = 0);
		static void DrawMesh(Ref<RenderPass>& renderPass, Ref<Mesh>& mesh);
		
		static void SetLineWidth(float width);

		struct GPUCapabilities
		{
			uint32_t MaxBoundTextures = INT_MAX;
			uint32_t MaxTextureMultisampling = 1;
			bool DynamicLineWidths = true;
			bool IndependentBlending = true;
		};
		static GPUCapabilities& GetGPUCapabilities();

		struct Statistics
		{
			uint32_t DrawCallCount;
		};
		static const Statistics& GetStats();

	};
};

