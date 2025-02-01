#pragma once

#include "Zahra/Renderer/IndexBuffer.h"
#include "Zahra/Renderer/Mesh.h"
#include "Zahra/Renderer/RenderPass.h"
#include "Zahra/Renderer/ShaderResourceManager.h"
#include "Zahra/Renderer/UniformBuffer.h"

#include <glm/glm.hpp>

namespace Zahra
{
	class RendererAPI
	{
	public:
		virtual ~RendererAPI() = default;

		enum class API
		{
			None = 0, OpenGL = 1, DX12 = 2, Vulkan = 3
		};

		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void OnWindowResize() = 0;

		virtual uint32_t GetSwapchainWidth() = 0;
		virtual uint32_t GetSwapchainHeight() = 0;
		virtual uint32_t GetFramesInFlight() = 0;
		virtual uint32_t GetCurrentFrameIndex() = 0;
		virtual uint32_t GetCurrentImageIndex() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void Present() = 0;

		inline static API GetAPI() { return s_API; }
		static RendererAPI* Create();
		
		// TODO: Break up resource bindings according to the following scheme:
		//		1) BeginRenderPass binds resources in sets 0 & 1
		//			(cameras, lights, shadow maps etc.)
		//		2) Each DrawXXX binds resources in sets 2 & 3
		//			(materials, textures, transforms)
		// Give RenderPass a ShaderResourceManager!

		virtual void BeginRenderPass(Ref<RenderPass>& renderPass, bool bindPipeline = true, bool clearAttachments = false) = 0;
		virtual void EndRenderPass() = 0;

		virtual void Draw(Ref<RenderPass>& renderPass, Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount) = 0;
		virtual void DrawIndexed(Ref<RenderPass>& renderPass, Ref<VertexBuffer>& vertexBuffer, Ref<IndexBuffer>& indexBuffer, uint32_t indexCount = 0, uint32_t startingIndex = 0) = 0;
		virtual void DrawMesh(Ref<RenderPass>& renderPass, Ref<StaticMesh>& mesh) = 0;
		virtual void DrawFullscreenTriangle(Ref<RenderPass>& renderPass) = 0;

		virtual void SetLineWidth(float width) = 0;

	private:
		static API s_API;

	};

}
