#pragma once

#include "Zahra/Renderer/RendererAPI.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace Zahra
{

	class VulkanRendererAPI : public RendererAPI
	{
	public:
		VulkanRendererAPI() = default;

		virtual void Init() override;
		virtual void Shutdown() override;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void SetClearColour(const glm::vec4& colour) override { m_ClearColour = { { { colour.r, colour.g, colour.b, colour.a } } }; }
		
		virtual void NewFrame() override;

		virtual void BeginRenderPass() override;
		virtual void EndRenderPass() override;
		virtual void SubmitCommandBuffer() override;

		virtual void BindPipeline(const Ref<Pipeline>& pipeline) override;

		virtual void Present() override;

		// TEMPORARY
		virtual void TutorialDrawCalls() override;

	private:
		Ref<VulkanSwapchain> m_Swapchain;

		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

		VkSemaphore m_ImageAvailableSemaphore;
		VkSemaphore m_RenderFinishedSemaphore;
		VkFence m_InFlightFence;

		VkClearValue m_ClearColour = { { { 0.05f, 0.1f, 0.05f, 1.0f } } };

		void CreateCommandPool();
		void AllocateCommandBuffer();

		void CreateSyncObjects();
	};

}
