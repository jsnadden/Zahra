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

		m_Format = VK_FORMAT_R8G8B8A8_SRGB;

		m_Width = width;
		m_Height = height;
		
		VkDeviceSize size = width * height * 4;  // 4 because currently we're just using 1 byte per channel

		Init(size);
		SetData((void*)pixelData, size);

		stbi_image_free(pixelData);
	}

	VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height)
	{
		m_Format = VK_FORMAT_R8G8B8A8_SRGB;

		m_Width = width;
		m_Height = height;

		VkDeviceSize size = width * height * 4;  // 4 because currently we're just using 1 byte per channel

		Init(size);
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
		VkDevice& device = VulkanContext::GetCurrentVkDevice();

		// TODO: employ VMA
		vkDestroySampler(device, m_Sampler, nullptr);
		vkDestroyImageView(device, m_ImageView, nullptr);
		vkDestroyImage(device, m_Image, nullptr);
		vkFreeMemory(device, m_ImageMemory, nullptr);
	}

	void VulkanTexture2D::SetData(void* data, uint32_t size)
	{
		m_ImageData.Write(data, size);

		Ref<VulkanDevice>& device = VulkanContext::GetCurrentDevice();
		VkDevice& vkDevice = device->GetVkDevice();

		//////////////////////////////////////////////////////////////////////////////////////////////////
		// CREATE IMAGE
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		device->CreateVulkanBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* mappedAddress;
		vkMapMemory(vkDevice, stagingBufferMemory, 0, size, 0, &mappedAddress);
		memcpy(mappedAddress, m_ImageData.GetData<void>(), size);
		vkUnmapMemory(vkDevice, stagingBufferMemory);

		device->CreateVulkanImage(m_Width, m_Height, m_Format, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_Image, m_ImageMemory);

		device->TransitionVulkanImageLayout(m_Image, m_Format,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		device->CopyVulkanBufferToImage(stagingBuffer, m_Image, m_Width, m_Height);

		device->TransitionVulkanImageLayout(m_Image, m_Format,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
		vkFreeMemory(vkDevice, stagingBufferMemory, nullptr);

		//////////////////////////////////////////////////////////////////////////////////////////////////
		// CREATE IMAGE VIEW

		m_ImageView = device->CreateVulkanImageView(m_Format, m_Image, VK_IMAGE_ASPECT_COLOR_BIT);

		//////////////////////////////////////////////////////////////////////////////////////////////////
		// CREATE SAMPLER

		VkFilter minFilter = VulkanUtils::TextureFilterModeToVkFilter(m_Specification.MinificationFilterMode);
		VkFilter magFilter = VulkanUtils::TextureFilterModeToVkFilter(m_Specification.MagnificationFilterMode);
		VkSamplerAddressMode addressMode = VulkanUtils::TextureAddressModeToVkSamplerAddressMode(m_Specification.AddressMode);
		
		m_Sampler = device->CreateVulkanImageSampler(minFilter, magFilter, addressMode);

		m_ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_ImageInfo.imageView = m_ImageView;
		m_ImageInfo.sampler = m_Sampler;

	}

	void VulkanTexture2D::Init(uint32_t size)
	{
		m_ImageData.Allocate(size);
		m_ImageData.ZeroInitialise();

	}

}
