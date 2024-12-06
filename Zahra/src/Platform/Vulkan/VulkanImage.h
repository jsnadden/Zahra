#pragma once

#include "Platform/Vulkan/VulkanContext.h"
#include "Zahra/Renderer/Image.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	namespace VulkanUtils
	{
		static VkImageUsageFlags VulkanImageUsage(ImageUsage usage)
		{
			switch (usage)
			{
			case ImageUsage::ColourAttachment:
				return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			case ImageUsage::DepthStencilAttachment:
				return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			case ImageUsage::Texture:
				return VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			}
			
			Z_CORE_ASSERT(false, "Unsupported image usage");
			return VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
		}

		static VkImageAspectFlags VulkanImageAspect(ImageUsage usage)
		{
			switch (usage)
			{
			case ImageUsage::ColourAttachment:
			case ImageUsage::Texture:
				return VK_IMAGE_ASPECT_COLOR_BIT;
			case ImageUsage::DepthStencilAttachment:
				return VK_IMAGE_ASPECT_DEPTH_BIT;
			}

			Z_CORE_ASSERT(false, "Unsupported image usage");
			return VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
		}

		static VkFormat GetColourFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::R8UN:				return VK_FORMAT_R8_UNORM;
			case ImageFormat::R8UI:				return VK_FORMAT_R8_UINT;
			case ImageFormat::R16UI:			return VK_FORMAT_R16_UINT;
			case ImageFormat::R32UI:			return VK_FORMAT_R32_UINT;
			case ImageFormat::R32F:				return VK_FORMAT_R32_SFLOAT;
			case ImageFormat::RG8:				return VK_FORMAT_R8G8_UNORM;
			case ImageFormat::RG16F:			return VK_FORMAT_R16G16_SFLOAT;
			case ImageFormat::RG32F:			return VK_FORMAT_R32G32_SFLOAT;
			case ImageFormat::RGBA:				return VK_FORMAT_R8G8B8A8_UNORM;
			case ImageFormat::SRGBA:			return VK_FORMAT_R8G8B8A8_SRGB;
			case ImageFormat::RGBA16F:			return VK_FORMAT_R16G16B16A16_SFLOAT;
			case ImageFormat::RGBA32F:			return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ImageFormat::B10R11G11UF:		return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
			}
			Z_CORE_ASSERT(false, "Unsupported colour format");
			return VK_FORMAT_UNDEFINED;
		}

		static VkFormat GetSupportedDepthStencilFormat()
		{
			return VulkanContext::GetCurrentDevice()->CheckFormatSupport({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
				VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		}

		static bool IsDepthFormat(VkFormat format)
		{
			switch (format)
			{
				case VK_FORMAT_D16_UNORM:
				case VK_FORMAT_D32_SFLOAT:
				case VK_FORMAT_D16_UNORM_S8_UINT:
				case VK_FORMAT_D24_UNORM_S8_UINT:
				case VK_FORMAT_D32_SFLOAT_S8_UINT:
				{
					return true;
					break;
				}
				default:
				{
					break;
				}
			}

			return false;
		}
	}

	// TODO: when I make an Image class as a rendering resource, a lot of this stuff can move there
	class VulkanImage : public Image
	{
	public:
		VulkanImage(uint32_t width, uint32_t height, ImageFormat format, ImageUsage usage);
		virtual ~VulkanImage() override;

		virtual const uint32_t GetWidth() const override { return m_Dimensions.width; }
		virtual const uint32_t GetHeight() const override { return m_Dimensions.height; }

		void TransitionLayout(VkImageLayout newLayout);

		void SetData(const VkBuffer& srcBuffer);

		const VkExtent2D GetExtent() const { return m_Dimensions; }
		const VkFormat& GetVkFormat() const { return m_Format; }
		const VkImageView& GetImageView() const { return m_ImageView; }


	private:
		VkExtent2D m_Dimensions;
		VkFormat m_Format;
		VkImageLayout m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageUsageFlags m_Usage;
		VkImage m_Image;
		VkDeviceMemory m_Memory;
		VkImageView m_ImageView;

	};
}
