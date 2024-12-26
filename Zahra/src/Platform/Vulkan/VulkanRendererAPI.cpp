#include "zpch.h"
#include "VulkanRendererAPI.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanIndexBuffer.h"
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
		return VulkanContext::Get()->GetSwapchain()->GetWidth();
	}

	uint32_t VulkanRendererAPI::GetSwapchainHeight()
	{
		return VulkanContext::Get()->GetSwapchain()->GetHeight();
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

		VkCommandBuffer& commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();

		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = 0;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		VulkanUtils::ValidateVkResult(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo),
			"Vulkan command buffer failed to begin recording");
		// TODO: reset descriptor pools?
	}

	void VulkanRendererAPI::EndFrame()
	{
		VkCommandBuffer commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();

		VulkanUtils::ValidateVkResult(vkEndCommandBuffer(commandBuffer),
			"Vulkan command buffer failed to end recording");
	}

	void VulkanRendererAPI::BeginRenderPass(Ref<RenderPass>& renderPass)
	{
		VkCommandBuffer& commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();

		Ref<VulkanRenderPass> vulkanRenderPass = renderPass.As<VulkanRenderPass>();
		std::vector<VkClearValue> clearValues = vulkanRenderPass->GetClearValues();
		VkExtent2D renderArea;

		if (vulkanRenderPass->TargetSwapchain())
		{
			if (m_Swapchain->Invalidated())
			{
				vulkanRenderPass->OnResize();
			}

			renderArea = m_Swapchain->GetExtent();
		}
		else
		{
			FramebufferSpecification renderTargetSpec = vulkanRenderPass->GetSpecification().RenderTarget->GetSpecification();
			renderArea = { renderTargetSpec.Width, renderTargetSpec.Height };			
		}

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = vulkanRenderPass->GetVkRenderPass();
		renderPassBeginInfo.framebuffer = vulkanRenderPass->GetVkFramebuffer();
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent.width = renderArea.width;
		renderPassBeginInfo.renderArea.extent.height = renderArea.height;
		renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanRenderPass->GetVkPipeline());

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(renderArea.width);
		viewport.height = static_cast<float>(renderArea.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = renderArea;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		// TODO: give RenderPass its own resource manager, and bind its descriptor sets here
		/*if (resourceManager)
		{
			auto vulkanResourceManager = resourceManager.As<VulkanShaderResourceManager>();
			auto& descriptorSets = vulkanResourceManager->GetDescriptorSets();
			uint32_t setCount = vulkanResourceManager->GetLastSet() - vulkanResourceManager->GetFirstSet() + 1;
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanRenderPass->GetVkPipelineLayout(),
				vulkanResourceManager->GetFirstSet(), setCount, descriptorSets.data(), 0, nullptr);
		}*/
	}

	void VulkanRendererAPI::EndRenderPass()
	{
		vkCmdEndRenderPass(m_Swapchain->GetCurrentDrawCommandBuffer());
	}

	void VulkanRendererAPI::Present()
	{
		m_Swapchain->PresentImage();			
	}

	void VulkanRendererAPI::Draw(Ref<RenderPass>& renderPass, Ref<ShaderResourceManager>& resourceManager, Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount)
	{
		VkCommandBuffer& commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();
		Ref<VulkanRenderPass> vulkanRenderPass = renderPass.As<VulkanRenderPass>();

		VkBuffer vulkanVertexBufferArray[] = { vertexBuffer.As<VulkanVertexBuffer>()->GetVulkanBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vulkanVertexBufferArray, offsets);

		resourceManager->Update();
		auto vulkanResourceManager = resourceManager.As<VulkanShaderResourceManager>();
		auto& descriptorSets = vulkanResourceManager->GetDescriptorSets();
		uint32_t setCount = vulkanResourceManager->GetLastSet() - vulkanResourceManager->GetFirstSet() + 1;
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanRenderPass->GetVkPipelineLayout(),
			vulkanResourceManager->GetFirstSet(), setCount, descriptorSets.data(), 0, nullptr);

		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	void VulkanRendererAPI::DrawIndexed(Ref<RenderPass>& renderPass, Ref<ShaderResourceManager>& resourceManager, Ref<VertexBuffer>& vertexBuffer, Ref<IndexBuffer>& indexBuffer, uint32_t indexCount, uint32_t startingIndex)
	{
		VkCommandBuffer& commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();
		Ref<VulkanRenderPass> vulkanRenderPass = renderPass.As<VulkanRenderPass>();

		VkBuffer vulkanVertexBufferArray[] = { vertexBuffer.As<VulkanVertexBuffer>()->GetVulkanBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vulkanVertexBufferArray, offsets);

		VkBuffer vulkanIndexBuffer = indexBuffer.As<VulkanIndexBuffer>()->GetVulkanBuffer();
		vkCmdBindIndexBuffer(commandBuffer, vulkanIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

		resourceManager->Update();
		auto vulkanResourceManager = resourceManager.As<VulkanShaderResourceManager>();
		auto& descriptorSets = vulkanResourceManager->GetDescriptorSets();
		uint32_t setCount = vulkanResourceManager->GetLastSet() - vulkanResourceManager->GetFirstSet() + 1;
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanRenderPass->GetVkPipelineLayout(),
			vulkanResourceManager->GetFirstSet(), setCount, descriptorSets.data(), 0, nullptr);

		if (indexCount == 0)
			indexCount = indexBuffer->GetCount() - startingIndex;

		vkCmdDrawIndexed(commandBuffer, indexCount, 1, startingIndex, 0, 0);
	}

	void VulkanRendererAPI::DrawMesh(Ref<RenderPass>& renderPass, Ref<ShaderResourceManager>& resourceManager, Ref<StaticMesh>& mesh)
	{
		VkCommandBuffer& commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();
		Ref<VulkanRenderPass> vulkanRenderPass = renderPass.As<VulkanRenderPass>();

		VkBuffer vulkanVertexBufferArray[] = { mesh->GetVertexBuffer().As<VulkanVertexBuffer>()->GetVulkanBuffer()};
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vulkanVertexBufferArray, offsets);

		VkBuffer vulkanIndexBuffer = mesh->GetIndexBuffer().As<VulkanIndexBuffer>()->GetVulkanBuffer();
		vkCmdBindIndexBuffer(commandBuffer, vulkanIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

		resourceManager->Update();
		auto vulkanResourceManager = resourceManager.As<VulkanShaderResourceManager>();
		auto& descriptorSets = vulkanResourceManager->GetDescriptorSets();
		uint32_t setCount = vulkanResourceManager->GetLastSet() - vulkanResourceManager->GetFirstSet() + 1;
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanRenderPass->GetVkPipelineLayout(),
			vulkanResourceManager->GetFirstSet(), setCount, descriptorSets.data(), 0, nullptr);

		vkCmdDrawIndexed(commandBuffer, (uint32_t)mesh->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

	void VulkanRendererAPI::DrawFullscreenTriangle(Ref<RenderPass>& renderPass, Ref<ShaderResourceManager>& resourceManager)
	{
		VkCommandBuffer& commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();
		Ref<VulkanRenderPass> vulkanRenderPass = renderPass.As<VulkanRenderPass>();

		resourceManager->Update();
		auto vulkanResourceManager = resourceManager.As<VulkanShaderResourceManager>();
		auto& descriptorSets = vulkanResourceManager->GetDescriptorSets();
		uint32_t setCount = vulkanResourceManager->GetLastSet() - vulkanResourceManager->GetFirstSet() + 1;
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanRenderPass->GetVkPipelineLayout(),
			vulkanResourceManager->GetFirstSet(), setCount, descriptorSets.data(), 0, nullptr);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	}

	void VulkanRendererAPI::SetLineWidth(float width)
	{
		VkCommandBuffer& commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();
		vkCmdSetLineWidth(commandBuffer, width);
	}
}
