#pragma once

#include "Platform/Vulkan/VulkanImage.h"
#include "Zahra/Core/Buffer.h"
#include "Zahra/Renderer/Texture.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	class VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(const Texture2DSpecification& specification, const std::filesystem::path& filepath);
		VulkanTexture2D(Ref<VulkanImage2D>& image);
		VulkanTexture2D(const Texture2DSpecification& specification, uint32_t colour);
		virtual ~VulkanTexture2D() override;

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual const std::filesystem::path& GetFilepath() const override { return m_Filepath; }
		virtual const Texture2DSpecification& GetSpecification() const override { return m_Specification; }

		// TODO: once we have a asset system in place this should be replaced with assetIDs
		virtual uint64_t GetHash() const { return (uint64_t)m_Image.As<VulkanImage2D>()->GetVkSampler(); }

		virtual void Resize(uint32_t width, uint32_t height) override;

		const VkImage& GetVkImage() const { return m_Image->GetVkImage(); }
		const VkImageView& GetVkImageView() const { return m_Image->GetVkImageView(); }
		const VkSampler& GetVkSampler() const { return m_Image->GetVkSampler(); }
		VkDescriptorImageInfo& GetVkDescriptorImageInfo() { return m_DescriptorImageInfo; }

	private:
		std::filesystem::path m_Filepath;
		Buffer m_LocalImageData;
		
		uint32_t m_Width, m_Height;
		uint32_t m_MipLevels;
		ImageFormat m_Format = ImageFormat::SRGBA;
		Texture2DSpecification m_Specification{};

		Ref<VulkanImage2D> m_Image;
		bool m_CreatedFromExistingImage = false;

		VkDescriptorImageInfo m_DescriptorImageInfo{};

		void SetData(void* data, uint32_t size);

	};
}
