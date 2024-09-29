//#pragma once
//
//#include "Platform/Vulkan/VulkanSwapchain.h"
//#include "Zahra/Renderer/RenderPass.h"
//
//namespace Zahra
//{
//	class VulkanRenderPass : public RenderPass
//	{
//	public:
//		VulkanRenderPass(RenderPassSpecification specification);
//		virtual ~VulkanRenderPass();
//
//		virtual RenderPassSpecification& GetSpecification() override { return m_Specification; }
//		virtual const RenderPassSpecification& GetSpecification() const override { return m_Specification; }
//
//		const VkRenderPass& GetVulkanRenderPassHandle() { return m_RenderPass; }
//		Ref<VulkanSwapchain> GetSwapchain() { return m_Swapchain; }
//
//	private:
//		RenderPassSpecification m_Specification;
//		Ref<VulkanSwapchain> m_Swapchain;
//		VkRenderPass m_RenderPass;
//
//	};
//}
//
