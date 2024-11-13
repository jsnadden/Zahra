#pragma once

#include "Zahra/Core/Buffer.h"
#include "Zahra/Renderer/UniformBuffer.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	class VulkanUniformBuffer : public UniformBuffer
	{
	public:
		VulkanUniformBuffer(uint64_t size);
		VulkanUniformBuffer(const void* data, uint64_t size, uint64_t offset);
		~VulkanUniformBuffer();

		virtual void SetData(const void* data, uint64_t size, uint64_t offset = 0) override;

	private:
		Buffer m_Data;
		VkBuffer m_VulkanBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_VulkanBufferMemory = VK_NULL_HANDLE;
		void* m_MappedAddress = nullptr;

		void Init(uint64_t size);
	};
}
