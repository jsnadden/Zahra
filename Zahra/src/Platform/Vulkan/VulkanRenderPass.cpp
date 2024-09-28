#include "zpch.h"
#include "VulkanRenderPass.h"

#include "Platform/Vulkan/VulkanContext.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	VulkanRenderPass::VulkanRenderPass(RenderPassSpecification specification)
		: m_Specification(specification)
	{
		WeakRef<VulkanContext> context = VulkanContext::Get();
		m_Swapchain = context->GetSwapchain();
		VkDevice device = context->GetCurrentDevice()->Device;

		// TODO: make plenty of this configurable

		// colour attachment for this render pass
		VkAttachmentDescription colourAttachment{};
		colourAttachment.format = m_Swapchain->GetImageFormat();
		colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // expected data layout of the image given to this render pass as input
		colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // data layout of the image this render pass will output

		// for now just a single subpass
		VkAttachmentReference colourAttachmentRef{};
		colourAttachmentRef.attachment = 0;
		colourAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // data layout the given subpass will treat the image as

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // as opposed to compute
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colourAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colourAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		VulkanUtils::ValidateVkResult(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass),
			"Vulkan render pass creation failed");

	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		VkDevice device = VulkanContext::Get()->GetCurrentDevice()->Device;
		vkDestroyRenderPass(device, m_RenderPass, nullptr);
	}
}
