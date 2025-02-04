#include "zpch.h"
#include "VulkanTexture.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanImGuiLayer.h"

namespace Zahra
{
	VulkanTexture2D::VulkanTexture2D(const Texture2DSpecification& specification, Buffer imageData)
		: m_Specification(specification)
	{
		Z_CORE_ASSERT(imageData, "Empty buffer");

		m_MipLevels = 1;
		if (specification.GenerateMips)
			m_MipLevels += (uint32_t)glm::floor(glm::log2((float)glm::max(m_Specification.Width, m_Specification.Height)));

		m_LocalImageData.Copy(imageData);
		CreateImageAndDescriptorInfo();
	}

	VulkanTexture2D::VulkanTexture2D(Ref<VulkanImage2D>& image)
		: m_Image(image)
	{
		Z_CORE_ASSERT(image->GetSpecification().Sampled);

		m_CreatedFromExistingImage = true;

		m_Specification.Format = image->GetSpecification().Format;
		m_Specification.Width = image->GetWidth();
		m_Specification.Height = image->GetHeight();

		m_MipLevels = 1;

		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorImageInfo.imageView = m_Image->GetVkImageView();
		m_DescriptorImageInfo.sampler = m_Image->GetVkSampler();
	}

	VulkanTexture2D::VulkanTexture2D(const Texture2DSpecification& specification, uint32_t colour)
		: m_Specification(specification)
	{
		Z_CORE_ASSERT(!m_Specification.GenerateMips, "Generating mips for a solid colour texture is very silly");
		m_MipLevels = 1;

		// TODO: extend to allow other formats. The buffer filling logic below will be more complex
		Z_CORE_ASSERT(!m_Specification.Format == ImageFormat::SRGBA);
		{
			uint64_t pixelCount = m_Specification.Width * m_Specification.Height;
			uint64_t pixelBytes = 4; // assuming srgba
			m_LocalImageData.Allocate(pixelCount * pixelBytes);
			for (uint64_t offset = 0; offset < pixelCount * pixelBytes; offset += pixelBytes)
			{
				m_LocalImageData.Write(&colour, pixelBytes, offset);
			}
		}

		CreateImageAndDescriptorInfo();
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
		VkDevice device = VulkanContext::GetCurrentVkDevice();
		vkDeviceWaitIdle(device);

		m_Image.Reset();
		m_LocalImageData.Release();
	}

	void VulkanTexture2D::Resize(uint32_t width, uint32_t height)
	{
		Z_CORE_ASSERT(m_CreatedFromExistingImage, "Only call this method for a texture created from an existing image");

		m_Specification.Width = width;
		m_Specification.Height = height;

		m_MipLevels = 1;

		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorImageInfo.imageView = m_Image->GetVkImageView();
		m_DescriptorImageInfo.sampler = m_Image->GetVkSampler();
	}

	void VulkanTexture2D::CreateImageAndDescriptorInfo()
	{
		Ref<VulkanDevice>& device = VulkanContext::GetCurrentDevice();
		VkDevice& vkDevice = device->GetVkDevice();

		///////////////////////////////////////////////////////////////////////////
		// Create staging buffer on device
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VkDeviceSize size = m_LocalImageData.GetSize();

		device->CreateVulkanBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		///////////////////////////////////////////////////////////////////////////
		// Copy local data to staging buffer
		void* mappedAddress;
		vkMapMemory(vkDevice, stagingBufferMemory, 0, size, 0, &mappedAddress);
		memcpy(mappedAddress, m_LocalImageData.GetData<void>(), size);
		vkUnmapMemory(vkDevice, stagingBufferMemory);

		///////////////////////////////////////////////////////////////////////////
		// Create sampled image
		Image2DSpecification spec{};
		spec.Width = m_Specification.Width;
		spec.Height = m_Specification.Height;
		spec.MipLevels = m_MipLevels;
		spec.Format = m_Specification.Format;
		spec.Sampled = true;
		spec.TransferSource = true;
		spec.TransferDestination = true;
		m_Image = Ref<VulkanImage2D>::Create(spec);
		m_Image->SetData(stagingBuffer); // this performs the copy operation, and the desired image layout transitions

		///////////////////////////////////////////////////////////////////////////
		// Cleanup
		vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
		vkFreeMemory(vkDevice, stagingBufferMemory, nullptr);

		if (!m_Specification.KeepLocalData)
			m_LocalImageData.Release();

		///////////////////////////////////////////////////////////////////////////
		// Populate descriptor info
		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorImageInfo.imageView = m_Image->GetVkImageView();
		m_DescriptorImageInfo.sampler = m_Image->GetVkSampler();
	}

}
