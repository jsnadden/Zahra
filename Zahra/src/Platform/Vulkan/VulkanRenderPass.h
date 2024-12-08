#pragma once


#include "Platform/Vulkan/VulkanContext.h"
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

		virtual RenderPassSpecification& GetSpecification() override { return m_Specification; }
		virtual const RenderPassSpecification& GetSpecification() const override { return m_Specification; }

		virtual Ref<Texture2D> GetOutputTexture() override;

		bool NeedsResizing();
		virtual void Refresh() override;

		const VkRenderPass& GetVkRenderPass() const { return m_RenderPass; }
		const VkPipeline& GetVkPipeline() const { return m_Pipeline; }
		const VkPipelineLayout& GetVkPipelineLayout() const { return m_PipelineLayout; }
		const VkExtent2D& GetAttachmentSize() const { return m_AttachmentSize; }
		const VkFramebuffer& GetFramebuffer(uint32_t index) const;
		const VkImage& GetPrimaryAttachmentVkImage() const { return m_PrimaryAttachment->GetVkImage(); }

	private:
		RenderPassSpecification m_Specification;
		Ref<VulkanSwapchain> m_Swapchain;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		void CreateRenderPass();

		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		void CreatePipeline();

		VkExtent2D m_AttachmentSize;
		Ref<VulkanImage> m_PrimaryAttachment; // only used if not targeting swapchain
		Ref<VulkanTexture2D> m_OutputTexture;
		Ref<VulkanImage> m_DepthStencilAttachment;
		std::vector<Ref<VulkanImage>> m_AdditionalAttachments; // TODO: implement this
		void CreateAttachments();
		void DestroyAttachments();

		std::vector<VkFramebuffer> m_Framebuffers;
		void CreateFramebuffers();
		void DestroyFramebuffers();

	};
}

