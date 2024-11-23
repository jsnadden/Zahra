#pragma once

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

		const VkDescriptorImageInfo& GetVkDescriptorImageInfo() { return m_ImageInfo; }


	private:
		Buffer m_ImageData;
		
		uint32_t m_Width, m_Height;
		Texture2DSpecification m_Specification;

		VkImage m_Image;
		VkFormat m_Format;
		VkDeviceMemory m_ImageMemory; // TODO: replace with vma
		VkImageView m_ImageView;
		VkSampler m_Sampler;

		VkDescriptorImageInfo m_ImageInfo;

		void Init(uint32_t size);
		
	};
}
