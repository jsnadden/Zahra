#include "zpch.h"
#include "VulkanCommandBuffer.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanPipeline.h"

namespace Zahra
{
	VulkanCommandBuffer::VulkanCommandBuffer(const CommandBufferSpecification& specification)
		: m_Specification(specification)
	{
		m_Swapchain = VulkanContext::Get()->GetSwapchain();
		CreateCommandPool();
		AllocateCommandBuffer();
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		VkDevice device = m_Swapchain->GetDevice()->GetVkDevice();

		vkDestroyCommandPool(device, m_CommandPool, nullptr);
	}

	void VulkanCommandBuffer::Record()
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = 0;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		VulkanUtils::ValidateVkResult(vkBeginCommandBuffer(m_CommandBuffer, &commandBufferBeginInfo),
			"Vulkan command buffer recording failed");

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = m_Swapchain->GetVkRenderPass();
		renderPassBeginInfo.framebuffer = m_Swapchain->GetCurrentFramebuffer();
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = m_Swapchain->GetExtent();

		VkClearValue clearColor = { { { 0.05f, 0.05f, 0.05f, 1.0f } } };
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;

		Ref<VulkanPipeline> pipeline = m_Specification.Pipeline;

		/////////////////////////////////////////////////////////////////////////////////////////////
		// SUBMIT COMMANDS TO BUFFER
		vkCmdBeginRenderPass(m_CommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkPipeline());

		// set dynamic state
		{
			auto& extent = m_Swapchain->GetExtent();
			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(extent.width);
			viewport.height = static_cast<float>(extent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(m_CommandBuffer, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = extent;
			vkCmdSetScissor(m_CommandBuffer, 0, 1, &scissor);
		}

		// FINALLY A DRAW CALL!!!
		vkCmdDraw(m_CommandBuffer, 3, 1, 0, 0);
		/////////////////////////////////////////////////////////////////////////////////////////////

		vkCmdEndRenderPass(m_CommandBuffer);

		VulkanUtils::ValidateVkResult(vkEndCommandBuffer(m_CommandBuffer),
			"Vulkan command buffer recording failed");

	}

	void VulkanCommandBuffer::CreateCommandPool()
	{		
		Ref<VulkanDevice> device = m_Swapchain->GetDevice();
		QueueFamilyIndices& queueFamilyIndices = device->GetQueueFamilyIndices();

		VkCommandPoolCreateInfo graphicsPoolInfo{};
		graphicsPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		graphicsPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		graphicsPoolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsIndex.value();

		VulkanUtils::ValidateVkResult(vkCreateCommandPool(device->GetVkDevice(), &graphicsPoolInfo, nullptr, &m_CommandPool),
			"Vulkan command pool creation failed");

	}

	void VulkanCommandBuffer::AllocateCommandBuffer()
	{
		Ref<VulkanDevice> device = m_Swapchain->GetDevice();

		VkCommandBufferAllocateInfo commandBufferInfo{};
		commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferInfo.commandPool = m_CommandPool;
		commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferInfo.commandBufferCount = 1;

		VulkanUtils::ValidateVkResult(vkAllocateCommandBuffers(device->GetVkDevice(), &commandBufferInfo, &m_CommandBuffer),
			"Vulkan command buffer allocation failed");

	}

}
