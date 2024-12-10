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

		// TODO: get/emplace clear values for additional attachments
		glm::vec3 clear = vulkanRenderPass->GetSpecification().PrimaryAttachment.ClearColour;
		std::vector<VkClearValue> clearValues = {{ clear.r, clear.g, clear.b, 1.0f }};
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

	void VulkanRendererAPI::EndRenderPass()
	{
		vkCmdEndRenderPass(m_Swapchain->GetCurrentDrawCommandBuffer());
	}

	//void VulkanRendererAPI::EndRenderPass(Ref<RenderPass> renderPass, Ref<Texture2D>& output)
	//{
	//	VkCommandBuffer commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();

	//	vkCmdEndRenderPass(commandBuffer);

	//	if (!renderPass->GetSpecification().OutputTexture)
	//		return;

	//	bool update = output;

	//	VkImage attachment = renderPass.As<VulkanRenderPass>()->GetPrimaryAttachmentVkImage();

	//	///////////////////////////////////////////////////////////////////////////////////////////////
	//	// TRANSITION PRIMARY ATTACHMENT TO TRANSFER SOURCE LAYOUT
	//	VkImageMemoryBarrier barrier{};
	//	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	//	barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	//	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	//	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // not transferring ownership
	//	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//	barrier.image = attachment;
	//	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//	barrier.subresourceRange.baseMipLevel = 0; // TODO: mipmapping
	//	barrier.subresourceRange.levelCount = 1; // TODO: mipmapping
	//	barrier.subresourceRange.baseArrayLayer = 0;
	//	barrier.subresourceRange.layerCount = 1;
	//	barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	//	barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

	//	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,					// dst stage
	//		0, 0, nullptr, 0, nullptr, 1, &barrier);

	//	VkImage outputImage;
	//	VkDeviceMemory outputMemory;
	//	VkImageLayout srcLayout;
	//	VkPipelineStageFlags srcStageMask;
	//	VkAccessFlags srcAccessMask;

	//	ImageFormat format = renderPass->GetSpecification().PrimaryAttachment.Format;
	//	uint32_t width = renderPass->GetSpecification().AttachmentWidth;
	//	uint32_t height = renderPass->GetSpecification().AttachmentHeight;

	//	if (update)
	//	{
	//		outputImage = output.As<VulkanTexture2D>()->GetVkImage();

	//		srcLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	//		srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	//		srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	//	}
	//	else
	//	{
	//		///////////////////////////////////////////////////////////////////////////////////////////////
	//		// CREATE NEW VULKAN IMAGE/MEMORY
	//		VkFormat vulkanFormat = VulkanUtils::GetColourFormat(format);
	//		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	//					
	//		VulkanContext::GetCurrentDevice()->CreateVulkanImage(width, height, vulkanFormat,
	//			VK_IMAGE_TILING_OPTIMAL, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outputImage, outputMemory);

	//		srcLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//		srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	//		srcAccessMask = 0;
	//	}		

	//	///////////////////////////////////////////////////////////////////////////////////////////////
	//	// TRANSITION OUTPUT IMAGE TO TRANSFER DESTINATION LAYOUT
	//	barrier.oldLayout = srcLayout;
	//	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	//	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // not transferring ownership
	//	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//	barrier.image = outputImage;
	//	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//	barrier.subresourceRange.baseMipLevel = 0; // TODO: mipmapping
	//	barrier.subresourceRange.levelCount = 1; // TODO: mipmapping
	//	barrier.subresourceRange.baseArrayLayer = 0;
	//	barrier.subresourceRange.layerCount = 1;
	//	barrier.srcAccessMask = srcAccessMask;
	//	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	//	vkCmdPipelineBarrier(commandBuffer, srcStageMask, VK_PIPELINE_STAGE_TRANSFER_BIT,
	//		0, 0, nullptr, 0, nullptr, 1, &barrier);

	//	///////////////////////////////////////////////////////////////////////////////////////////////
	//	// COPY ATTACHMENT DATA TO NEW IMAGE
	//	VkImageCopy copyRegion{};
	//	copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//	copyRegion.srcSubresource.mipLevel = 0; // TODO: mipmapping
	//	copyRegion.srcSubresource.baseArrayLayer = 0;
	//	copyRegion.srcSubresource.layerCount = 1;
	//	copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//	copyRegion.dstSubresource.mipLevel = 0; // TODO: mipmapping
	//	copyRegion.dstSubresource.baseArrayLayer = 0;
	//	copyRegion.dstSubresource.layerCount = 1;
	//	copyRegion.extent.width = width;
	//	copyRegion.extent.height = height;
	//	copyRegion.extent.depth = 1;

	//	vkCmdCopyImage(commandBuffer, attachment, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, outputImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

	//	///////////////////////////////////////////////////////////////////////////////////////////////
	//	// TRANSITION OUTPUT IMAGE TO TEXTURE LAYOUT
	//	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	//	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	//	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // not transferring ownership
	//	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//	barrier.image = outputImage;
	//	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//	barrier.subresourceRange.baseMipLevel = 0; // TODO: mipmapping
	//	barrier.subresourceRange.levelCount = 1; // TODO: mipmapping
	//	barrier.subresourceRange.baseArrayLayer = 0;
	//	barrier.subresourceRange.layerCount = 1;
	//	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	//	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	//	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	//		0, 0, nullptr, 0, nullptr, 1, &barrier);

	//	if (!update)
	//	{
	//		///////////////////////////////////////////////////////////////////////////////////////////////
	//		// CREATE OUTPUT TEXTURE
	//		ImageSpecification outputImageSpecification{};
	//		outputImageSpecification.Format = format;
	//		outputImageSpecification.Width = width;
	//		outputImageSpecification.Height = height;
	//		outputImageSpecification.Usage = ImageUsage::Texture;

	//		output = Ref<VulkanTexture2D>::Create(outputImageSpecification.Width, outputImageSpecification.Height);
	//		output->SetData(Ref<VulkanImage>::Create(outputImage, outputMemory, outputImageSpecification));
	//	}

	//}

	void VulkanRendererAPI::Present()
	{
		m_Swapchain->PresentImage();			
	}

	void VulkanRendererAPI::DrawIndexed(Ref<RenderPass> renderPass, Ref<ShaderResourceManager> resourceManager, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, uint32_t indexCount, uint32_t startingIndex)
	{
		VkCommandBuffer commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();

		VkBuffer vulkanVertexBufferArray[] = { vertexBuffer.As<VulkanVertexBuffer>()->GetVulkanBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vulkanVertexBufferArray, offsets);

		VkBuffer vulkanIndexBuffer = indexBuffer.As<VulkanIndexBuffer>()->GetVulkanBuffer();
		vkCmdBindIndexBuffer(commandBuffer, vulkanIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

		// TODO: set the dynamic state based on the renderPass attachment size?
		// actually these should really be moved to BeginRenderPass!!
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

		vkCmdDrawIndexed(commandBuffer, indexCount, 1, startingIndex, 0, 0);
	}

	void VulkanRendererAPI::DrawMesh(Ref<RenderPass> renderPass, Ref<ShaderResourceManager> resourceManager, Ref<StaticMesh> mesh)
	{
		VkCommandBuffer commandBuffer = m_Swapchain->GetCurrentDrawCommandBuffer();

		VkBuffer vulkanVertexBufferArray[] = { mesh->GetVertexBuffer().As<VulkanVertexBuffer>()->GetVulkanBuffer()};
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vulkanVertexBufferArray, offsets);

		VkBuffer vulkanIndexBuffer = mesh->GetIndexBuffer().As<VulkanIndexBuffer>()->GetVulkanBuffer();
		vkCmdBindIndexBuffer(commandBuffer, vulkanIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

		// TODO: set the dynamic state based on the renderPass attachment size?
		// actually these should really be moved to BeginRenderPass!!
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

		vkCmdDrawIndexed(commandBuffer, (uint32_t)mesh->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

}

