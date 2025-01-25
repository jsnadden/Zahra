#pragma once

#include "Platform/Vulkan/VulkanContext.h"
#include "Zahra/Renderer/Image.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	namespace VulkanUtils
	{
		static VkFormat GetSupportedDepthStencilFormat()
		{
			return VulkanContext::GetCurrentDevice()->GetSupportingFormat(
				{
					VK_FORMAT_D32_SFLOAT_S8_UINT,
					VK_FORMAT_D32_SFLOAT,
					VK_FORMAT_D24_UNORM_S8_UINT
				},
				VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		}

		static VkFormat VulkanFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::R8_UN:			return VK_FORMAT_R8_UNORM;
				case ImageFormat::R8_UI:			return VK_FORMAT_R8_UINT;
				case ImageFormat::R16_UI:			return VK_FORMAT_R16_UINT;
				case ImageFormat::R32_UI:			return VK_FORMAT_R32_UINT;
				case ImageFormat::R32_SI:			return VK_FORMAT_R32_SINT;
				case ImageFormat::R32_F:			return VK_FORMAT_R32_SFLOAT;
				case ImageFormat::RG8_UN:			return VK_FORMAT_R8G8_UNORM;
				case ImageFormat::RG16_F:			return VK_FORMAT_R16G16_SFLOAT;
				case ImageFormat::RG32_F:			return VK_FORMAT_R32G32_SFLOAT;
				case ImageFormat::RGBA_UN:			return VK_FORMAT_R8G8B8A8_UNORM;
				case ImageFormat::SRGBA:			return VK_FORMAT_R8G8B8A8_SRGB;
				case ImageFormat::RGBA16_F:			return VK_FORMAT_R16G16B16A16_SFLOAT;
				case ImageFormat::RGBA32_F:			return VK_FORMAT_R32G32B32A32_SFLOAT;
				case ImageFormat::B10R11G11_UF:		return VK_FORMAT_B10G11R11_UFLOAT_PACK32;

				case ImageFormat::DepthStencil:		return GetSupportedDepthStencilFormat();
			}
			Z_CORE_ASSERT(false, "Unsupported image format");
			return VK_FORMAT_UNDEFINED;
		}

		static bool IsIntegerFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::R8_UI:
				case ImageFormat::R16_UI:
				case ImageFormat::R32_UI:
				case ImageFormat::R32_SI:
					return true;

				case ImageFormat::R8_UN:
				case ImageFormat::R32_F:
				case ImageFormat::RG8_UN:
				case ImageFormat::RG16_F:
				case ImageFormat::RG32_F:
				case ImageFormat::RGBA_UN:
				case ImageFormat::SRGBA:
				case ImageFormat::RGBA16_F:
				case ImageFormat::RGBA32_F:
				case ImageFormat::B10R11G11_UF:
				case ImageFormat::DepthStencil:
					return false;
			}
			Z_CORE_ASSERT(false, "Unsupported image format");
			return false;
		}

		static VkImageLayout VulkanImageLayout(ImageLayout layout)
		{
			switch (layout)
			{
				case ImageLayout::Unspecified: return VK_IMAGE_LAYOUT_UNDEFINED;
				case ImageLayout::ColourAttachment: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				case ImageLayout::DepthStencilAttachment: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				case ImageLayout::Texture: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				case ImageLayout::TransferSource: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				case ImageLayout::TransferDestination: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				case ImageLayout::Presentation: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			}
			Z_CORE_ASSERT(false, "Unsupported image layout");
			return VK_IMAGE_LAYOUT_MAX_ENUM;
		}

		static VkAccessFlags AccessFlagForLayoutTransition(ImageLayout layout)
		{
			switch (layout)
			{
				case ImageLayout::ColourAttachment: return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
				case ImageLayout::DepthStencilAttachment: return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
				case ImageLayout::Texture: return VK_ACCESS_SHADER_READ_BIT;
				case ImageLayout::TransferSource: return VK_ACCESS_TRANSFER_READ_BIT;
				case ImageLayout::TransferDestination: return VK_ACCESS_TRANSFER_WRITE_BIT;
				case ImageLayout::Unspecified: return 0;
			}
			Z_CORE_ASSERT(false, "Unsupported image layout");
			return VK_ACCESS_FLAG_BITS_MAX_ENUM;
		}

		static VkPipelineStageFlags StageFlagForLayoutTransition(ImageLayout layout)
		{
			switch (layout)
			{
				case ImageLayout::ColourAttachment: return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				case ImageLayout::DepthStencilAttachment: return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				case ImageLayout::Texture: return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				case ImageLayout::TransferSource: return VK_PIPELINE_STAGE_TRANSFER_BIT;
				case ImageLayout::TransferDestination: return VK_PIPELINE_STAGE_TRANSFER_BIT;
				case ImageLayout::Unspecified: return 0;
			}
			Z_CORE_ASSERT(false, "Unsupported image layout");
			return VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM;
		}
	}

	class VulkanImage2D : public Image2D
	{
	public:
		VulkanImage2D() = default;
		VulkanImage2D(Image2DSpecification specification);
		virtual ~VulkanImage2D() override;

		virtual const Image2DSpecification GetSpecification() const override { return m_Specification; }
		virtual const uint32_t GetWidth() const override { return m_Specification.Width; }
		virtual const uint32_t GetHeight() const override { return m_Specification.Height; }

		virtual void Resize(uint32_t width, uint32_t height) override;

		void SetData(const VkBuffer& srcBuffer);

		virtual void* ReadPixel(int32_t x, int32_t y) override;

		const VkExtent2D GetDimensions() const { return { m_Specification.Width, m_Specification.Height }; }
		VkFormat GetVkFormat() const { return VulkanUtils::VulkanFormat(m_Specification.Format); }
		const VkImageView& GetVkImageView() const { return m_ImageView; }
		const VkImage& GetVkImage() const { return m_Image; }
		const VkSampler& GetVkSampler() const { return m_Sampler; }
		const VkImageLayout& GetExpectedLayout() const { return VulkanUtils::VulkanImageLayout(m_CurrentLayout); }

	private:
		Image2DSpecification m_Specification;
		
		VkImage m_Image = VK_NULL_HANDLE;
		VkDeviceMemory m_Memory = VK_NULL_HANDLE;
		VkImageView m_ImageView = VK_NULL_HANDLE;

		VkSampler m_Sampler = VK_NULL_HANDLE;

		VkBuffer m_PixelBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_PixelBufferMemory = VK_NULL_HANDLE;
		void* m_PixelBufferMappedAddress = nullptr;

		ImageLayout m_CurrentLayout = ImageLayout::Unspecified;

		void Init();
		void Cleanup();

		void CreateAndAllocateImage();
		void GenerateMips();
		void CreateImageView();
		void CreateSampler();
		void CreatePixelBuffer();

		/*void TransitionLayout(ImageLayout from, ImageLayout to);
		void TransitionLayout(ImageLayout to) { TransitionLayout(m_CurrentLayout, to); }*/
	};
}
