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
		virtual const Ref<Framebuffer> GetRenderTarget() const override;

		virtual bool TargetSwapchain() override;

		const VkRenderPass& GetVkRenderPass() const { return m_RenderPass; }
		const VkPipeline& GetVkPipeline() const { return m_Pipeline; }
		const VkPipelineLayout& GetVkPipelineLayout() const { return m_PipelineLayout; }
		const VkFramebuffer& GetVkFramebuffer() const;

		const std::vector<VkClearValue> GetClearValues() const;
		const std::vector<VkClearAttachment>& GetClearAttachments() const { return m_ClearAttachments; }
		const std::vector<VkClearRect>& GetClearRects() const { return m_ClearRects; }

		virtual void OnResize() override;

	private:
		RenderPassSpecification m_Specification;
		Ref<VulkanSwapchain> m_Swapchain;
		bool m_TargetSwapchain = false;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_Pipeline = VK_NULL_HANDLE;

		std::vector<VkFramebuffer> m_Framebuffers;

		std::vector<VkClearValue> m_ClearValues;
		std::vector<VkClearAttachment> m_ClearAttachments;
		std::vector<VkClearRect> m_ClearRects;

		void CreateRenderPass();
		void CreatePipeline();
		void CreateFramebuffers();
		void DestroyFramebuffers();

		void CreateClearData();

		void ValidateSpecification();

	};
}

