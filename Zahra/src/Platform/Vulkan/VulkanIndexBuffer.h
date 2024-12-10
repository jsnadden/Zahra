#pragma once

#include "Zahra/Core/Buffer.h"
#include "Zahra/Renderer/IndexBuffer.h"

#include "vulkan/vulkan.h"

namespace Zahra
{
	class VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(const uint32_t* indices, uint64_t count);
		~VulkanIndexBuffer();

		virtual void SetData(const uint32_t* data, uint64_t size) override;

		virtual uint64_t GetCount() const { return m_IndexCount; }

		VkBuffer GetVulkanBuffer() { return m_VulkanIndexBuffer; }

	private:
		Buffer m_IndexData;
		uint64_t m_IndexCount;

		VkBuffer m_VulkanIndexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_VulkanIndexBufferMemory = VK_NULL_HANDLE;

	};
}
