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

		//virtual void Record() override;

	private:
		CommandBufferSpecification m_Specification;
		// TODO: do we need a different pool/buffer for each GPUQueueType?
		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

		Ref<VulkanSwapchain> m_Swapchain;

		void CreateCommandPool();
		void AllocateCommandBuffer();

	};
}
