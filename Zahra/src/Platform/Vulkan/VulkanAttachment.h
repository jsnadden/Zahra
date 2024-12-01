#pragma once

#include "Platform/Vulkan/VulkanContext.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	namespace VulkanUtils
	{
		static VkAttachmentLoadOp VulkanLoadOp(AttachmentLoadOp op)
		{
			switch (op)
			{
			case AttachmentLoadOp::Load:
			{
				return VK_ATTACHMENT_LOAD_OP_LOAD;
				break;
			}
			case AttachmentLoadOp::Clear:
			{
				return VK_ATTACHMENT_LOAD_OP_CLEAR;
				break;
			}
			case AttachmentLoadOp::Unspecified:
			{
				return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				break;
			}

			}

			Z_CORE_ASSERT(false, "Unrecognised LoadOp");
			return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
		}

		static VkAttachmentStoreOp VulkanStoreOp(AttachmentStoreOp op)
		{
			switch (op)
			{
			case AttachmentStoreOp::Store:
			{
				return VK_ATTACHMENT_STORE_OP_STORE;
				break;
			}
			case AttachmentStoreOp::Unspecified:
			{
				return VK_ATTACHMENT_STORE_OP_DONT_CARE;
				break;
			}

			}

			Z_CORE_ASSERT(false, "Unrecognised StoreOp");
			return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
		}

		static VkImageLayout VulkanAttachmentLayout(AttachmentLayout layout)
		{
			switch (layout)
			{
			case AttachmentLayout::Undefined:
			{
				return VK_IMAGE_LAYOUT_UNDEFINED;
				break;
			}
			case AttachmentLayout::Colour:
			{
				return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			}
			case AttachmentLayout::DepthStencil:
			{
				return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			}
			case AttachmentLayout::Present:
			{
				return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				break;
			}
			}

			Z_CORE_ASSERT(false, "Unrecognised layout");
			return VK_IMAGE_LAYOUT_MAX_ENUM;
		}

		static VkFormat VulkanAttachmentFormat(AttachmentFormat format)
		{
			switch (format)
			{
			case AttachmentFormat::R8UN:			return VK_FORMAT_R8_UNORM;
			case AttachmentFormat::R8UI:			return VK_FORMAT_R8_UINT;
			case AttachmentFormat::R16UI:			return VK_FORMAT_R16_UINT;
			case AttachmentFormat::R32UI:			return VK_FORMAT_R32_UINT;
			case AttachmentFormat::R32F:			return VK_FORMAT_R32_SFLOAT;
			case AttachmentFormat::RG8:				return VK_FORMAT_R8G8_UNORM;
			case AttachmentFormat::RG16F:			return VK_FORMAT_R16G16_SFLOAT;
			case AttachmentFormat::RG32F:			return VK_FORMAT_R32G32_SFLOAT;
			case AttachmentFormat::RGBA:			return VK_FORMAT_R8G8B8A8_UNORM;
			case AttachmentFormat::SRGBA:			return VK_FORMAT_R8G8B8A8_SRGB;
			case AttachmentFormat::RGBA16F:			return VK_FORMAT_R16G16B16A16_SFLOAT;
			case AttachmentFormat::RGBA32F:			return VK_FORMAT_R32G32B32A32_SFLOAT;
			case AttachmentFormat::B10R11G11UF:		return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
			}
			Z_CORE_ASSERT(false, "Unrecognised format");
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

	class VulkanAttachment : public RefCounted
	{
	public:
		VulkanAttachment(VkExtent2D size, VkFormat format, VkImageUsageFlags usage);
		~VulkanAttachment();

		const VkFormat& GetVkFormat() const { return m_Format; }
		const VkImageView& GetImageView() const { return m_ImageView; }


	private:
		VkFormat m_Format;
		VkImage m_Image;
		VkDeviceMemory m_Memory;
		VkImageView m_ImageView;

	};
}
