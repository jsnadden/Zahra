#include "zpch.h"
#include "VulkanRendererAPI.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanPipeline.h"

namespace Zahra
{

	void VulkanRendererAPI::Init()
	{
		m_Swapchain = VulkanContext::Get()->GetSwapchain();

		CreateCommandPool();
		AllocateCommandBuffer();

		CreateSyncObjects();
	}

	void VulkanRendererAPI::Shutdown()
	{
		VkDevice device = m_Swapchain->GetLogicalDevice();

		vkDestroySemaphore(device, m_ImageAvailableSemaphore, nullptr);
		vkDestroySemaphore(device, m_RenderFinishedSemaphore, nullptr);
		vkDestroyFence(device, m_InFlightFence, nullptr);
		
		vkDestroyCommandPool(device, m_CommandPool, nullptr);
	}

	void VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		// TODO: figure out which class "owns" this and forward it this data
	}

	void VulkanRendererAPI::NewFrame()
	{
		vkWaitForFences(m_Swapchain->GetLogicalDevice(), 1, &m_InFlightFence, VK_TRUE, UINT64_MAX);
		vkResetFences(m_Swapchain->GetLogicalDevice(), 1, &m_InFlightFence);

		m_Swapchain->GetNextImage(m_ImageAvailableSemaphore);

		vkResetCommandBuffer(m_CommandBuffer, 0);

		// TODO: reset descriptor pools
	}

	void VulkanRendererAPI::BeginRenderPass()
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = 0;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		VulkanUtils::ValidateVkResult(vkBeginCommandBuffer(m_CommandBuffer, &commandBufferBeginInfo),
			"Vulkan command buffer failed to begin recording");

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = m_Swapchain->GetVkRenderPass();
		renderPassBeginInfo.framebuffer = m_Swapchain->GetCurrentFramebuffer();
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = m_Swapchain->GetExtent();
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &m_ClearColour;

		vkCmdBeginRenderPass(m_CommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		
	}

	void VulkanRendererAPI::EndRenderPass()
	{
		vkCmdEndRenderPass(m_CommandBuffer);

		VulkanUtils::ValidateVkResult(vkEndCommandBuffer(m_CommandBuffer),
			"Vulkan command buffer failed to end recording");
	}

	void VulkanRendererAPI::SubmitCommandBuffer()
	{
		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphore };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		VulkanUtils::ValidateVkResult(vkQueueSubmit(m_Swapchain->GetDevice()->GraphicsQueue, 1, &submitInfo, m_InFlightFence));
	}

	void VulkanRendererAPI::BindPipeline(const Ref<Pipeline>& pipeline)
	{
		vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.As<VulkanPipeline>()->GetVkPipeline());
	}

	void VulkanRendererAPI::Present()
	{

	}

	void VulkanRendererAPI::TutorialDrawCalls()
	{
		// set dynamic state
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

		// FINALLY A DRAW CALL!!!
		vkCmdDraw(m_CommandBuffer, 3, 1, 0, 0);
	}

	void VulkanRendererAPI::CreateCommandPool()
	{
		VkCommandPoolCreateInfo graphicsPoolInfo{};
		graphicsPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		graphicsPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		graphicsPoolInfo.queueFamilyIndex = m_Swapchain->GetDevice()->QueueFamilyIndices.GraphicsIndex.value();

		VulkanUtils::ValidateVkResult(vkCreateCommandPool(m_Swapchain->GetLogicalDevice(), &graphicsPoolInfo, nullptr, &m_CommandPool),
			"Vulkan command pool creation failed");
	}

	void VulkanRendererAPI::AllocateCommandBuffer()
	{
		VkCommandBufferAllocateInfo commandBufferInfo{};
		commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferInfo.commandPool = m_CommandPool;
		commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferInfo.commandBufferCount = 1;

		VulkanUtils::ValidateVkResult(vkAllocateCommandBuffers(m_Swapchain->GetLogicalDevice(), &commandBufferInfo, &m_CommandBuffer),
			"Vulkan command buffer allocation failed");
	}

	void VulkanRendererAPI::CreateSyncObjects()
	{
		// these guys have trivial CreateInfos
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		// fence starts in the signaled state, otherwise we would wait indefinitely on the first frame:
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkDevice device = m_Swapchain->GetLogicalDevice();

		VulkanUtils::ValidateVkResult(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore),
			"Vulkan semaphore creation failed");
		VulkanUtils::ValidateVkResult(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore),
			"Vulkan semaphore creation failed");
		VulkanUtils::ValidateVkResult(vkCreateFence(device, &fenceInfo, nullptr, &m_InFlightFence),
			"Vulkan fence creation failed");

	}

}

