#pragma once

#include "Zahra/Core/Buffer.h"
#include "Zahra/Renderer/UniformBuffer.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	class VulkanUniformBuffer : public UniformBuffer
	{
	public:
		VulkanUniformBuffer(uint32_t size);
		VulkanUniformBuffer(const void* data, uint32_t size, uint32_t offset);
		~VulkanUniformBuffer();

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

		VkDescriptorBufferInfo& GetVkDescriptorBufferInfo() { return m_BufferInfo; }

	private:
		Buffer m_Data;
		VkBuffer m_VulkanBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_VulkanBufferMemory = VK_NULL_HANDLE;
		void* m_MappedAddress = nullptr;

		VkDescriptorBufferInfo m_BufferInfo{};

		void Init(uint32_t size);

	};

	class VulkanUniformBufferPerFrame : public UniformBufferPerFrame
	{
	public:
		VulkanUniformBufferPerFrame(uint32_t bufferSize, uint32_t framesInFlight);
		~VulkanUniformBufferPerFrame() { m_UniformBuffers.clear(); }

		virtual Ref<UniformBuffer> Get();
		virtual Ref<UniformBuffer> Get(uint32_t frame) override { return m_UniformBuffers[frame].As<UniformBuffer>(); }
		virtual void Set(uint32_t frame, Ref<UniformBuffer> buffer) override { m_UniformBuffers[frame] = buffer.As<VulkanUniformBuffer>(); }
		virtual void SetData(uint32_t frame, const void* data, uint32_t size, uint32_t offset = 0) override;

	private:
		uint32_t m_FramesInFlight;
		// TODO: instead of multiple buffers, replace this with a single buffer and use offsets
		std::map<uint32_t, Ref<VulkanUniformBuffer>> m_UniformBuffers;

	};

}
