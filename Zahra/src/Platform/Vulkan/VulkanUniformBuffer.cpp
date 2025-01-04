#include "zpch.h"
#include "VulkanUniformBuffer.h"

#include "Platform/Vulkan/VulkanContext.h"

namespace Zahra
{
	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size)
	{
		Init(size);
	}

	VulkanUniformBuffer::VulkanUniformBuffer(const void* data, uint32_t size, uint32_t offset)
	{
		Init(size);
		SetData(data, size, offset);
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();

		vkDeviceWaitIdle(device);

		vkDestroyBuffer(device, m_VulkanBuffer, nullptr);
		vkFreeMemory(device, m_VulkanBufferMemory, nullptr);

		m_Data.Release();
	}

	void VulkanUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		m_Data.Write(data, size, offset);

		memcpy(m_MappedAddress, m_Data.GetData<void>(), size);
	}

	void VulkanUniformBuffer::Init(uint32_t size)
	{
		m_Data.Allocate(size);
		m_Data.ZeroInitialise();

		Ref<VulkanDevice> device = VulkanContext::GetCurrentDevice();

		device->CreateVulkanBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_VulkanBuffer, m_VulkanBufferMemory);

		vkMapMemory(device->GetVkDevice(), m_VulkanBufferMemory, 0, size, 0, &m_MappedAddress);

		m_BufferInfo.buffer = m_VulkanBuffer;
		m_BufferInfo.offset = 0;
		m_BufferInfo.range = size;
	}

	VulkanUniformBufferSet::VulkanUniformBufferSet(uint32_t bufferSize, uint32_t framesInFlight)
	{
		if (framesInFlight == 0)
			m_FramesInFlight = Renderer::GetFramesInFlight();
		else
			m_FramesInFlight = framesInFlight;

		for (uint32_t frame = 0; frame < framesInFlight; frame++)
			m_UniformBuffers[frame] = Ref<VulkanUniformBuffer>::Create(bufferSize);

	}

	Ref<UniformBuffer> VulkanUniformBufferSet::Get()
	{
		uint32_t frameIndex = Renderer::GetCurrentFrameIndex();
		return Get(frameIndex);
	}

	void VulkanUniformBufferSet::SetData(uint32_t frame, const void* data, uint32_t size, uint32_t offset)
	{
		m_UniformBuffers[frame]->SetData(data, size, offset);
	}

}
