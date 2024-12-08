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
		VulkanTexture2D(const Texture2DSpecification& specification);
		VulkanTexture2D(uint32_t width, uint32_t height);
		virtual ~VulkanTexture2D() override;

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual const std::filesystem::path& GetFilepath() const override { return m_Specification.ImageFilepath; }
		virtual const Texture2DSpecification& GetSpecification() const override { return m_Specification; }

		virtual void SetData(void* data, uint32_t size) override;
		virtual void SetData(Ref<Image> srcImage) override;

		const VkImageView& GetVkImageView() const { return m_Image->GetVkImageView(); }
		const VkSampler& GetSampler() const { return m_Sampler; }
		const VkDescriptorImageInfo& GetVkDescriptorImageInfo() const { return m_DescriptorImageInfo; }


	private:
		Buffer m_LocalImageData;
		
		uint32_t m_Width, m_Height;
		Texture2DSpecification m_Specification;

		Ref<VulkanImage> m_Image;
		VkSampler m_Sampler = VK_NULL_HANDLE;
		VkDescriptorImageInfo m_DescriptorImageInfo;

		void InitialiseLocalBuffer(uint32_t dataSize);
		void CreateImage();
		void CreateSampler();
		void CreateDescriptorImageInfo();
		
	};
}
