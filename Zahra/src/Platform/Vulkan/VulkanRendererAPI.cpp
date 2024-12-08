#include "zpch.h"
#include "VulkanRendererAPI.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanIndexBuffer.h"
#include "Platform/Vulkan/VulkanPipeline.h"
#include "Platform/Vulkan/VulkanShaderResourceManager.h"
#include "Platform/Vulkan/VulkanVertexBuffer.h"

namespace Zahra
{

	void VulkanRendererAPI::Init()
	{
		m_Swapchain = VulkanContext::Get()->GetSwapchain();
		m_Device = m_Swapchain->GetDevice();
		m_FramesInFlight = m_Swapchain->GetFramesInFlight();		
	}

	void VulkanRendererAPI::Shutdown()
	{
		
	}

	void VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		//VulkanContext::Get()->GetSwapchain()->OnWindowResize(width, height);
	}

	void VulkanRendererAPI::OnWindowResize()
	{
		VulkanContext::Get()->GetSwapchain()->SignalResize();
	}

	uint32_t VulkanRendererAPI::GetSwapchainWidth()
	{
		return VulkanContext::Get()->GetSwapchain()->GetExtent().width;
	}

	uint32_t VulkanRendererAPI::GetSwapchainHeight()
	{
		return VulkanContext::Get()->GetSwapchain()->GetExtent().height;
	}

	uint32_t VulkanRendererAPI::GetFramesInFlight()
	{
		return VulkanContext::Get()->GetSwapchain()->GetFramesInFlight();
	}

	uint32_t VulkanRendererAPI::GetCurrentFrameIndex()
	{
		return VulkanContext::Get()->GetSwapchain()->GetFrameIndex();
	}

	uint32_t VulkanRendererAPI::GetCurrentImageIndex()
	{
		return VulkanContext::Get()->GetSwapchain()->GetImageIndex();
	}

	void VulkanRendererAPI::BeginFrame()
	{
		m_Swapchain->GetNextImage();

		VkCommandBuffer commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();

		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = 0;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		VulkanUtils::ValidateVkResult(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo),
			"Vulkan command buffer failed to begin recording");
		// TODO: reset descriptor pools
	}

	void VulkanRendererAPI::EndFrame()
	{
		VkCommandBuffer commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();

		VulkanUtils::ValidateVkResult(vkEndCommandBuffer(commandBuffer),
			"Vulkan command buffer failed to end recording");
	}

	void VulkanRendererAPI::BeginRenderPass(Ref<RenderPass> renderpass)
	{
		VkCommandBuffer commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();

		Ref<VulkanRenderPass> vulkanRenderPass = renderpass.As<VulkanRenderPass>();
		if (m_Swapchain->Invalidated()) vulkanRenderPass->Refresh();

		std::vector<VkClearValue> clearValues = { m_ClearColour };
		// TODO: add clear values for other attachments
		if (vulkanRenderPass->GetSpecification().HasDepthStencil) clearValues.emplace_back(m_ClearDepthStencil);

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = vulkanRenderPass->GetVkRenderPass();
		renderPassBeginInfo.framebuffer = vulkanRenderPass->GetFramebuffer(m_Swapchain->GetImageIndex());
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = vulkanRenderPass->GetAttachmentSize();
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanRenderPass->GetVkPipeline());
	}

	void VulkanRendererAPI::EndRenderPass(Ref<RenderPass> renderPass)
	{
		VkCommandBuffer commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();

		vkCmdEndRenderPass(commandBuffer);

		if (renderPass->GetSpecification().OutputTexture)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // not transferring ownership
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = renderPass.As<VulkanRenderPass>()->GetPrimaryAttachmentVkImage();
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0; // TODO: mipmapping
			barrier.subresourceRange.levelCount = 1; // TODO: mipmapping
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	// src stage
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,			// dst stage
				0,												// dependency flags
				0, nullptr,										// memory barriers (irrelevant)
				0, nullptr,										// buffer memory barriers (irrelevant)
				1, &barrier										// image memory barriers (this is the one!!)
			);
		}
	}

	void VulkanRendererAPI::Present()
	{
		m_Swapchain->PresentImage();			
	}

	void VulkanRendererAPI::TutorialDrawCalls(Ref<RenderPass> renderPass, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, Ref<ShaderResourceManager> resourceManager)
	{
		VkCommandBuffer commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();

		VkBuffer vulkanVertexBufferArray[] = { vertexBuffer.As<VulkanVertexBuffer>()->GetVulkanBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vulkanVertexBufferArray, offsets);

		VkBuffer vulkanIndexBuffer = indexBuffer.As<VulkanIndexBuffer>()->GetVulkanBuffer();
		vkCmdBindIndexBuffer(commandBuffer, vulkanIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

		// set dynamic state
		auto& extent = m_Swapchain->GetExtent();
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		auto vulkanRenderPass = renderPass.As<VulkanRenderPass>();
		auto vulkanResourceManager = resourceManager.As<VulkanShaderResourceManager>();
		auto& descriptorSets = vulkanResourceManager->GetDescriptorSets();
		uint32_t setCount = vulkanResourceManager->GetLastSet() - vulkanResourceManager->GetFirstSet() + 1;
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanRenderPass->GetVkPipelineLayout(), vulkanResourceManager->GetFirstSet(), setCount, descriptorSets.data(), 0, nullptr);

		// FINALLY A DRAW CALL!!!
		vkCmdDrawIndexed(commandBuffer, (uint32_t)indexBuffer->GetCount(), 1, 0, 0, 0);
	}

	void VulkanRendererAPI::TutorialDrawCalls(Ref<RenderPass> renderPass, Ref<StaticMesh> mesh, Ref<ShaderResourceManager> resourceManager)
	{
		VkCommandBuffer commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();

		VkBuffer vulkanVertexBufferArray[] = { mesh->GetVertexBuffer().As<VulkanVertexBuffer>()->GetVulkanBuffer()};
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vulkanVertexBufferArray, offsets);

		VkBuffer vulkanIndexBuffer = mesh->GetIndexBuffer().As<VulkanIndexBuffer>()->GetVulkanBuffer();
		vkCmdBindIndexBuffer(commandBuffer, vulkanIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

		// set dynamic state
		auto& extent = m_Swapchain->GetExtent();
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		auto vulkanRenderPass = renderPass.As<VulkanRenderPass>();
		auto vulkanResourceManager = resourceManager.As<VulkanShaderResourceManager>();
		auto& descriptorSets = vulkanResourceManager->GetDescriptorSets();
		uint32_t setCount = vulkanResourceManager->GetLastSet() - vulkanResourceManager->GetFirstSet() + 1;
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanRenderPass->GetVkPipelineLayout(), vulkanResourceManager->GetFirstSet(), setCount, descriptorSets.data(), 0, nullptr);

		// FINALLY A DRAW CALL!!!
		vkCmdDrawIndexed(commandBuffer, (uint32_t)mesh->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

}

