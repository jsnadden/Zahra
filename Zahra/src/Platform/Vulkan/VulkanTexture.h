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
		VulkanTexture2D(const TextureSpecification& specification, Buffer imageData);
		VulkanTexture2D(Ref<VulkanImage2D>& image);
		VulkanTexture2D(const TextureSpecification& specification, uint32_t colour);
		virtual ~VulkanTexture2D() override;

		virtual const TextureSpecification& GetSpecification() const override { return m_Specification; }
		virtual uint32_t GetWidth() const override { return m_Specification.Width; }
		virtual uint32_t GetHeight() const override { return m_Specification.Height; }

		virtual void Resize(uint32_t width, uint32_t height) override;

		const VkImage& GetVkImage() const { return m_Image->GetVkImage(); }
		const VkImageView& GetVkImageView() const { return m_Image->GetVkImageView(); }
		const VkSampler& GetVkSampler() const { return m_Image->GetVkSampler(); }
		VkDescriptorImageInfo& GetVkDescriptorImageInfo() { return m_DescriptorImageInfo; }

	private:
		Buffer m_LocalImageData;
		
		uint32_t m_MipLevels;
		TextureSpecification m_Specification{};

		Ref<VulkanImage2D> m_Image;
		bool m_CreatedFromExistingImage = false;

		VkDescriptorImageInfo m_DescriptorImageInfo{};

		void CreateImageAndDescriptorInfo();

	};
}
