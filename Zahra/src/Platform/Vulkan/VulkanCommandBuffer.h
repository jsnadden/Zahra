#pragma once

#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Zahra/Renderer/CommandBuffer.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		VulkanCommandBuffer(const CommandBufferSpecification& specification);
		virtual ~VulkanCommandBuffer();

		// TODO: this will be heavily refactored across the Zahra::Renderer system
		virtual void Record() override;

	private:
		CommandBufferSpecification m_Specification;
		// TODO: do we need a different pool/buffer for each GPUQueueType?
		Ref<VulkanSwapchain> m_Swapchain;
		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

		void CreateCommandPool();
		void AllocateCommandBuffer();

	};
}
