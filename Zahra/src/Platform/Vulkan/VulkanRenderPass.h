#pragma once


#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanImage.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Zahra/Renderer/RenderPass.h"

#include "vulkan/vulkan.h"

namespace Zahra
{
	namespace VulkanUtils
	{
		static VkPrimitiveTopology VulkanTopology(PrimitiveTopology topology)
		{
			switch (topology)
			{
				case PrimitiveTopology::Points:
				{
					return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
					break;
				}
				case PrimitiveTopology::Lines:
				{
					return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
					break;
				}
				case PrimitiveTopology::LineStrip:
				{
					return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
					break;
				}
				case PrimitiveTopology::Triangles:
				{
					return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
					break;
				}
				case PrimitiveTopology::TriangleStrip:
				{
					return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
					break;
				}
				case PrimitiveTopology::TriangleFan:
				{
					return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
					break;
				}
			}

			Z_CORE_ASSERT(false, "Unrecognised topology");
			return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
		}
	}


	class VulkanRenderPass : public RenderPass
	{
	public:
		VulkanRenderPass(const RenderPassSpecification& specification);
		~VulkanRenderPass();

		virtual const RenderPassSpecification& GetSpecification() const override { return m_Specification; }
		virtual const Ref<Framebuffer> GetRenderTarget() const { return m_Specification.RenderTarget; }

		virtual void Resize(uint32_t width, uint32_t height) override { } // TODO: this would only be necessary if we target the swapchain

		const VkRenderPass& GetVkRenderPass() const { return m_RenderPass; }
		const VkPipeline& GetVkPipeline() const { return m_Pipeline; }
		const VkPipelineLayout& GetVkPipelineLayout() const { return m_PipelineLayout; }
		const VkFramebuffer& GetVkFramebuffer() { return m_Framebuffer; }

		const std::vector<VkClearValue> const GetClearValues() { return m_Specification.RenderTarget.As<VulkanFramebuffer>()->GetClearValues(); }

	private:
		RenderPassSpecification m_Specification;
		Ref<VulkanSwapchain> m_Swapchain;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_Pipeline = VK_NULL_HANDLE;

		VkFramebuffer m_Framebuffer;

		void CreateRenderPass();
		void CreatePipeline();
		void CreateFramebuffer();

		void ValidateSpecification();

	};
}

