#include "zpch.h"
#include "VulkanTexture.h"

#include "Platform/Vulkan/VulkanContext.h"

#include <stb_image.h>

namespace Zahra
{
	namespace VulkanUtils
	{
		VkFilter VulkanFilterMode(TextureFilterMode mode)
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

		VkSamplerAddressMode VulkanAddressMode(TextureWrapMode mode)
		{
			switch (mode)
			{
			case TextureWrapMode::Repeat:
			{
				return VK_SAMPLER_ADDRESS_MODE_REPEAT;
				break;
			}
			case TextureWrapMode::MirroredRepeat:
			{
				return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
				break;
			}
			case TextureWrapMode::ClampToEdge:
			{
				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				break;
			}
			case TextureWrapMode::ClampToBorder:
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

		uint32_t BytesPerPixel(ImageFormat format)
		{
			switch (format)
			{
				//case ImageFormat::R8_UN:				return
				//case ImageFormat::R8_UI:				return
				//case ImageFormat::R16_UI:				return
				//case ImageFormat::R32_UI:				return
				//case ImageFormat::R32_F:				return
				//case ImageFormat::RG8_UN:				return
				//case ImageFormat::RG16_F:				return
				//case ImageFormat::RG32_F:				return
				//case ImageFormat::RGBA_UN:			return;
				case ImageFormat::SRGBA:				return 4;
				//case ImageFormat::RGBA16_F:			return
				//case ImageFormat::RGBA32_F:			return
				//case ImageFormat::B10R11G11_UF:		return
				//case ImageFormat::DepthStencil:		return
			}
			Z_CORE_ASSERT(false, "Unsupported image format");
			return VK_FORMAT_UNDEFINED;
		}
	}

	VulkanTexture2D::VulkanTexture2D(const Texture2DSpecification& specification, std::filesystem::path filepath)
		: m_Specification(specification)
	{
		int width, height, channels;

		stbi_uc* imageData = stbi_load(filepath.string().c_str(), &width, &height, &channels, 4);

		Z_CORE_ASSERT(imageData, "Vulkan texture failed to load image.");

		// TODO: to allow for hdr textures etc., stbi can query the image file to decide on a correct colour format
		m_Format = ImageFormat::SRGBA;
		m_Format = ImageFormat::SRGBA;
		m_Width = width;
		m_Height = height;

		VkDeviceSize size = width * height * VulkanUtils::BytesPerPixel(m_Format);
		SetData((void*)imageData, size);

		stbi_image_free(imageData);
	}

	VulkanTexture2D::VulkanTexture2D(const Ref<VulkanImage2D>& image)
		: m_Image(image)
	{
		Z_CORE_ASSERT(image->GetSpecification().Sampled);

		m_CreatedFromExistingImage = true;

		m_Format = image->GetSpecification().Format;
		m_Width = image->GetWidth();
		m_Height = image->GetHeight();

		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorImageInfo.imageView = m_Image->GetVkImageView();
		m_DescriptorImageInfo.sampler = m_Image->GetVkSampler();
	}

	VulkanTexture2D::VulkanTexture2D(const Texture2DSpecification& specification, uint32_t colour)
		: m_Specification(specification)
	{
		m_Format = ImageFormat::SRGBA;
		m_Width = 1;
		m_Height = 1;

		VkDeviceSize size = VulkanUtils::BytesPerPixel(m_Format);
		SetData((void*)&colour, size);
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

		m_Width = width;
		m_Height = height;

		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorImageInfo.imageView = m_Image->GetVkImageView();
		m_DescriptorImageInfo.sampler = m_Image->GetVkSampler();
	}

	void VulkanTexture2D::SetData(void* data, uint32_t size)
	{
		///////////////////////////////////////////////////////////////////////////
		// Create local buffer
		m_LocalImageData.Allocate(size);
		m_LocalImageData.ZeroInitialise();
		m_LocalImageData.Write(data, size);

		Ref<VulkanDevice>& device = VulkanContext::GetCurrentDevice();
		VkDevice& vkDevice = device->GetVkDevice();

		///////////////////////////////////////////////////////////////////////////
		// Create staging buffer on device
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

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
		ImageSpecification spec{};
		spec.Width = m_Width;
		spec.Height = m_Height;
		spec.Format = m_Format;
		spec.Sampled = true;
		spec.TransferSource = false; // TODO: might want to add to this the spec?
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
