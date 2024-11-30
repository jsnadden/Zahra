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
		if (m_Swapchain->Invalidated()) vulkanRenderPass->RefreshFramebuffers();

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = vulkanRenderPass->GetVkRenderPass();
		renderPassBeginInfo.framebuffer = vulkanRenderPass->GetFramebuffer(m_Swapchain->GetImageIndex());
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = m_Swapchain->GetExtent();
		renderPassBeginInfo.clearValueCount = vulkanRenderPass->GetSpecification().HasDepthStencil ? 2 : 1;
		renderPassBeginInfo.pClearValues = m_ClearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanRenderPass->GetVkPipeline());
	}

	void VulkanRendererAPI::EndRenderPass()
	{
		VkCommandBuffer commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();

		vkCmdEndRenderPass(commandBuffer);
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

	void VulkanRendererAPI::TutorialDrawCalls(Ref<RenderPass> renderPass, Ref<Mesh> mesh, Ref<ShaderResourceManager> resourceManager)
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

