#include "zpch.h"
#include "VulkanTexture.h"

#include "VulkanContext.h"

#include <stb_image.h>

namespace Zahra
{
	VulkanTexture2D::VulkanTexture2D(const std::filesystem::path& filepath)
		: m_Filepath(filepath)
	{
		int width, height, channels;

		stbi_uc* pixelData = stbi_load(filepath.string().c_str(), &width, &height, &channels, 4);
		// TODO: add possibility of loading hdr images (stbi_loadf)

		Z_CORE_ASSERT(pixelData, "Vulkan texture failed to load image.");

		m_Width = width;
		m_Height = height;
		
		VkDeviceSize size = width * height * 4;

		Init(size);
		SetData((void*)pixelData, size);

		stbi_image_free(pixelData);
	}

	VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height)
	{
		m_Width = width;
		m_Height = height;

		Init(width * height * 4);
	}

	VulkanTexture2D::~VulkanTexture2D()
	{

	}

	void VulkanTexture2D::SetData(void* data, uint32_t size)
	{
		m_ImageData.Write(data, size);

		VkDevice& device = VulkanContext::GetCurrentVkDevice();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		VulkanContext::GetCurrentDevice()->CreateVulkanBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* mappedAddress;
		vkMapMemory(device, stagingBufferMemory, 0, size, 0, &mappedAddress);
		memcpy(mappedAddress, m_ImageData.GetData<void>(), size);
		vkUnmapMemory(device, stagingBufferMemory);

		VulkanContext::GetCurrentDevice()->CreateVulkanImage2D(m_Width, m_Height,
			VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VulkanImage, m_VulkanImageMemory);

	}

	void VulkanTexture2D::Init(uint32_t size)
	{
		m_ImageData.Allocate(size);
		m_ImageData.ZeroInitialise();
	}

}
