#include "zpch.h"
#include "Texture.h"

#include "Platform/Vulkan/VulkanTexture.h"
#include "Zahra/Renderer/Renderer.h"

#include <stb_image.h>

namespace Zahra
{
	Ref<Texture2D> Texture2D::CreateFromBuffer(const TextureSpecification& specification, Buffer imageData)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::OpenGL:	Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
			case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
			case RendererAPI::API::Vulkan:	return Ref<VulkanTexture2D>::Create(specification, imageData);
		}

		Z_CORE_ASSERT(false, "Invalid RendererAPI::API value");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::CreateFromImage2D(Ref<Image2D>& image)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::OpenGL:	Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
			case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
			case RendererAPI::API::Vulkan:	return Ref<VulkanTexture2D>::Create(image.As<VulkanImage2D>());
		}

		Z_CORE_ASSERT(false, "Invalid RendererAPI::API value");
		return nullptr;
	}

    Ref<Texture2D> Texture2D::CreateFlatColourTexture(const TextureSpecification& specification, uint32_t colour)
    {
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::OpenGL:	Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
			case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
			case RendererAPI::API::Vulkan:	return Ref<VulkanTexture2D>::Create(specification, colour);
		}

        Z_CORE_ASSERT(false, "Invalid RendererAPI::API value");
        return nullptr;
    }

	Ref<Texture2D> TextureLoader::LoadTexture2DAsset(const AssetHandle& handle, const AssetMetadata& metadata)
	{
		return LoadTexture2DFromSource(metadata.Filepath, true);
	}

	Ref<Texture2D> TextureLoader::LoadTexture2DFromSource(const std::filesystem::path& sourceFilepath, bool generateMips)
	{
		TextureSpecification spec{};
		Buffer imageData = LoadImageData(sourceFilepath, spec.Width, spec.Height, spec.Format);
		spec.GenerateMips = generateMips;

		Ref<Texture2D> newTexture = Texture2D::CreateFromBuffer(spec, imageData);

		stbi_image_free(imageData.Data);
		return newTexture;
	}

	Buffer TextureLoader::LoadImageData(const std::filesystem::path& sourceFilepath, uint32_t& widthOut, uint32_t& heightOut, ImageFormat& formatOut)
	{
		int width = 0, height = 0, channels = 4;
		bool validfilepath = std::filesystem::exists(sourceFilepath);
		std::string filepathString = sourceFilepath.string();
		stbi_uc* imageData = nullptr;

		if (stbi_is_hdr(filepathString.c_str()))
		{
			// TODO: add this capability
			Z_CORE_ASSERT(false, "Not currently supporting HDR texture formats")
				/*format = ;
				imageData = stbi_loadf();*/
		}
		else
		{
			imageData = stbi_load(filepathString.c_str(), &width, &height, &channels, channels);

			if (!imageData)
				return nullptr;

			widthOut = width;
			heightOut = height;

			switch (channels)
			{
				case 3:
				{
					formatOut = ImageFormat::SRGB;
					break;
				}
				case 4:
				{
					formatOut = ImageFormat::SRGBA;
					break;
				}
				default:
				{
					Z_CORE_ASSERT(false, "Currently only support these formats")
						break;
				}
			}
		}

		Buffer buffer((void*)imageData, (uint64_t)(width * height * Image::BytesPerPixel(formatOut)));
		return buffer;
	}

}

