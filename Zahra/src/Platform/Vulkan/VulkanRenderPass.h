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
		virtual const Ref<Framebuffer> GetFramebuffer() const override;

		//virtual Ref<Texture2D> GetOutputTexture() override;

		//bool NeedsResizing();
		virtual void Resize(uint32_t width, uint32_t height) override;

		const VkRenderPass& GetVkRenderPass() const { return m_RenderPass; }
		const VkPipeline& GetVkPipeline() const { return m_Pipeline; }
		const VkPipelineLayout& GetVkPipelineLayout() const { return m_PipelineLayout; }
		const VkFramebuffer& GetVkFramebuffer();

		const std::vector<VkClearValue> const GetClearValues();

	private:
		RenderPassSpecification m_Specification;
		Ref<VulkanSwapchain> m_Swapchain;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_Pipeline = VK_NULL_HANDLE;

		Ref<VulkanFramebuffer> m_Framebuffer;
		std::vector<VkFramebuffer> m_VkFramebuffers;

		void CreateRenderPass();
		void CreatePipeline();
		void CreateFramebuffer();
		void DestroyFramebuffer();

		void ValidateSpecification();

		std::vector<VkAttachmentDescription> GenerateAttachmentDescriptions();
		std::vector<VkAttachmentReference> GenerateColourAttachmentReferences();

	};
}

