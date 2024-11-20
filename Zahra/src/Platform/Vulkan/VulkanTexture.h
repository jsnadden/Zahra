#pragma once

#include "Zahra/Core/Buffer.h"
#include "Zahra/Renderer/Texture.h"

#include <vulkan/vulkan.h>

namespace Zahra
{
	class VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(const std::filesystem::path& filepath);
		VulkanTexture2D(uint32_t width, uint32_t height);
		virtual ~VulkanTexture2D() override;

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual const std::filesystem::path& GetFilepath() const override { return m_Filepath; }

		virtual void SetData(void* data, uint32_t size) override;


	private:
		Buffer m_ImageData;
		
		uint32_t m_Width, m_Height;
		std::filesystem::path m_Filepath;

		VkImage m_VulkanImage;
		VkDeviceMemory m_VulkanImageMemory;

		void Init(uint32_t size);
		
	};
}
