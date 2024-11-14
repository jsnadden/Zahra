#pragma once

#include "Zahra/Core/Buffer.h"
#include "Zahra/Renderer/VertexBuffer.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		// TODO: there should be an additional argument to choose static/dynamic drawing
		VulkanVertexBuffer(uint64_t size);
		VulkanVertexBuffer(const void* data, uint64_t size);
		~VulkanVertexBuffer();

		virtual void SetData(const void* data, uint64_t size) override;

		VkBuffer GetVulkanBuffer() { return m_VulkanVertexBuffer; }

	private:
		Buffer m_VertexData; // TODO: do I need local storage?
		VkBuffer m_VulkanVertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_VulkanVertexBufferMemory = VK_NULL_HANDLE;

		void Init(uint64_t size);

	};
}
