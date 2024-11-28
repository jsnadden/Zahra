#pragma once

#include "Zahra/Renderer/RenderPass.h"

#include "Platform/Vulkan/VulkanContext.h"

#include "vulkan/vulkan.h"

namespace Zahra
{
	
	// TODO: currently assuming we're only rendering to swapchain images, as opposed to an
	// auxilliary framebuffer, so I should generalise this class, or provide alternatives
	class VulkanRenderPass : public RenderPass
	{
	public:
		VulkanRenderPass(const RenderPassSpecification& specification);
		~VulkanRenderPass();

		virtual RenderPassSpecification& GetSpecification() override { return m_Specification; }
		virtual const RenderPassSpecification& GetSpecification() const override { return m_Specification; }

		bool NeedsResizing();
		virtual void RefreshFramebuffers() override;

		const VkRenderPass& GetVkRenderPass() { return m_RenderPass; }
		const VkPipeline& GetVkPipeline() { return m_Pipeline; }
		const VkPipelineLayout& GetVkPipelineLayout() { return m_PipelineLayout; }
		const VkFramebuffer& GetFramebuffer(uint32_t index);

	private:
		RenderPassSpecification m_Specification;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		void CreateRenderPass();

		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		void CreatePipeline();

		std::vector<VkFramebuffer> m_Framebuffers;
		VkExtent2D m_FramebufferSize;
		void CreateFramebuffers();
		void DestroyFramebuffers();

		Ref<VulkanSwapchain> m_Swapchain;

	};
}

