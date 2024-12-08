#include "zpch.h"
#include "VulkanTexture.h"

#include "Platform/Vulkan/VulkanContext.h"

#include <stb_image.h>

namespace Zahra
{
	namespace VulkanUtils
	{
		VkFilter TextureFilterModeToVkFilter(TextureFilterMode mode)
		{
			switch (mode)
			{
				case TextureFilterMode::Nearest:
				{
					return VK_FILTER_NEAREST;
					break;
				}
				case TextureFilterMode::Linear:
				{
					return VK_FILTER_LINEAR;
					break;
				}
				default:
					break;
			}
			Z_CORE_ASSERT(false, "Unrecognised texture filtering mode");
			return VK_FILTER_MAX_ENUM;
		}

		VkSamplerAddressMode TextureAddressModeToVkSamplerAddressMode(TextureAddressMode mode)
		{
			switch (mode)
			{
			case TextureAddressMode::Repeat:
			{
				return VK_SAMPLER_ADDRESS_MODE_REPEAT;
				break;
			}
			case TextureAddressMode::MirroredRepeat:
			{
				return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
				break;
			}
			case TextureAddressMode::ClampToEdge:
			{
				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				break;
			}
			case TextureAddressMode::ClampToBorder:
			{
				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
				break;
			}
			default:
				break;
			}
			Z_CORE_ASSERT(false, "Unrecognised texture tiling mode");
			return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
		}
	}

	VulkanTexture2D::VulkanTexture2D(const Texture2DSpecification& specification)
		: m_Specification(specification)
	{
		int width, height, channels;

		// TODO: to allow for hdr textures, stbi can query the image metadata, then we'd want to use
		// stbi_loadf and a float format instead of VK_FORMAT_R8G8B8A8_SRGB

		stbi_uc* pixelData = stbi_load(specification.ImageFilepath.string().c_str(), &width, &height, &channels, 4);

		Z_CORE_ASSERT(pixelData, "Vulkan texture failed to load image.");

		m_Width = width;
		m_Height = height;

		VkDeviceSize size = width * height * 4;  // 4 because currently we're just using 1 byte per channel

		InitialiseLocalBuffer(size);
		SetData((void*)pixelData, size);

		stbi_image_free(pixelData);
	}

	VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height)
	{
		m_Width = width;
		m_Height = height;

		VkDeviceSize size = width * height * 4;  // 4 because currently we're just using 1 byte per channel

		InitialiseLocalBuffer(size);
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
		// TODO: employ VMA
		vkDestroySampler(VulkanContext::GetCurrentVkDevice(), m_Sampler, nullptr);
		
		m_Image.Reset();

		m_LocalImageData.Release();
	}

	void VulkanTexture2D::SetData(void* data, uint32_t size)
	{
		CreateImage();

		m_LocalImageData.Write(data, size);

		Ref<VulkanDevice>& device = VulkanContext::GetCurrentDevice();
		VkDevice& vkDevice = device->GetVkDevice();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		device->CreateVulkanBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* mappedAddress;
		vkMapMemory(vkDevice, stagingBufferMemory, 0, size, 0, &mappedAddress);
		memcpy(mappedAddress, m_LocalImageData.GetData<void>(), size);
		vkUnmapMemory(vkDevice, stagingBufferMemory);

		m_Image->TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		m_Image->SetData(stagingBuffer);
		m_Image->TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
		vkFreeMemory(vkDevice, stagingBufferMemory, nullptr);

		CreateSampler();
		CreateDescriptorImageInfo();
	}

	void VulkanTexture2D::SetData(Ref<Image> srcImage)
	{
		m_Width = srcImage->GetWidth();
		m_Height = srcImage->GetHeight();
		m_Image = srcImage.As<VulkanImage>();

		CreateSampler();
		CreateDescriptorImageInfo();
	}

	void VulkanTexture2D::InitialiseLocalBuffer(uint32_t dataSize)
	{
		m_LocalImageData.Allocate(dataSize);
		m_LocalImageData.ZeroInitialise();
	}

	void VulkanTexture2D::CreateImage()
	{
		ImageSpecification spec{};
		spec.Format = ImageFormat::SRGBA; // TODO: for hdr textures should use a float format instead
		spec.Width = m_Width;
		spec.Height = m_Height;
		spec.Usage = ImageUsage::Texture;

		m_Image = Ref<VulkanImage>::Create(spec);
	}

	void VulkanTexture2D::CreateSampler()
	{
		if (m_Sampler != VK_NULL_HANDLE)
		{
			// TODO: employ VMA
			vkDestroySampler(VulkanContext::GetCurrentVkDevice(), m_Sampler, nullptr);
		}

		VkFilter minFilter = VulkanUtils::TextureFilterModeToVkFilter(m_Specification.MinificationFilterMode);
		VkFilter magFilter = VulkanUtils::TextureFilterModeToVkFilter(m_Specification.MagnificationFilterMode);
		VkSamplerAddressMode addressMode = VulkanUtils::TextureAddressModeToVkSamplerAddressMode(m_Specification.AddressMode);

		m_Sampler = VulkanContext::GetCurrentDevice()->CreateVulkanImageSampler(minFilter, magFilter, addressMode);
	}

	void VulkanTexture2D::CreateDescriptorImageInfo()
	{
		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorImageInfo.imageView = m_Image->GetVkImageView();
		m_DescriptorImageInfo.sampler = m_Sampler;
	}

}
