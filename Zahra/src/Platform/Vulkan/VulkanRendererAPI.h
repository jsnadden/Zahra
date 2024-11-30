#pragma once

#include "Zahra/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanPipeline.h"
#include "Platform/Vulkan/VulkanRenderPass.h"

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

		virtual void BeginRenderPass(Ref<RenderPass> renderPass) override;
		virtual void EndRenderPass() override;

		virtual void Present() override;

		// TEMPORARY
		virtual void TutorialDrawCalls(Ref<RenderPass> renderPass, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, Ref<ShaderResourceManager> resourceManager) override;
		virtual void TutorialDrawCalls(Ref<RenderPass> renderPass, Ref<Mesh> mesh, Ref<ShaderResourceManager> resourceManager) override;

	private:
		Ref<VulkanSwapchain> m_Swapchain;
		Ref<VulkanDevice> m_Device;

		uint32_t m_FramesInFlight;

		std::vector<VkClearValue> m_ClearValues = { { { 0.0f, 0.0f, 0.0f, 1.0f } } , { 1.0f, 0 } };
				
	};

}
