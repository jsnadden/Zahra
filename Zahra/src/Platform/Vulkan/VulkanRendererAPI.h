#pragma once

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Zahra/Renderer/RendererAPI.h"

namespace Zahra
{

	class VulkanRendererAPI : public RendererAPI
	{
	public:
		VulkanRendererAPI() = default;

		virtual void Init() override;
		virtual void Shutdown() override;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void OnWindowResize() override;

		virtual uint32_t GetSwapchainWidth() override;
		virtual uint32_t GetSwapchainHeight() override;
		virtual uint32_t GetFramesInFlight() override;
		virtual uint32_t GetCurrentFrameIndex() override;
		virtual uint32_t GetCurrentImageIndex() override;

		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		virtual void BeginRenderPass(Ref<RenderPass> renderPass, Ref<ShaderResourceManager> resourceManager) override;
		virtual void EndRenderPass() override;
		//virtual void EndRenderPass(Ref<RenderPass> renderPass, Ref<Texture2D>& output) override;

		virtual void Present() override;

		virtual void DrawIndexed(Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, uint64_t indexCount = 0, uint64_t startingIndex = 0) override;
		virtual void DrawMesh(Ref<StaticMesh> mesh) override;

		virtual void FinalDrawCall() override;

	private:
		Ref<VulkanSwapchain> m_Swapchain;
		Ref<VulkanDevice> m_Device;

		uint32_t m_FramesInFlight;
	};

}
