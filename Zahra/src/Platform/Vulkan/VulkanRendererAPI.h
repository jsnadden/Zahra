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

		virtual void BeginRenderPass(Ref<Pipeline> pipeline) override;
		virtual void EndRenderPass() override;

		virtual void PresentImage() override;

		// TEMPORARY
		virtual void TutorialDrawCalls() override;

	private:
		Ref<VulkanSwapchain> m_Swapchain;

		VkClearValue m_ClearColour = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };

	};

}
