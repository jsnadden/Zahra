#include "zpch.h"
#include "Texture.h"

#include "Platform/Vulkan/VulkanTexture.h"
#include "Zahra/Renderer/Renderer.h"

#include <stb_image.h>

namespace Zahra
{

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

    Ref<Texture2D> Texture2D::CreateFlatColourTexture(const Texture2DSpecification& specification, uint32_t colour)
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

	Ref<Texture2D> Texture2D::LoadTexture2DFromSource(const AssetHandle& handle, const AssetMetadata& metadata)
	{
		int width = 0, height = 0, channels = 4;
		bool validfilepath = std::filesystem::exists(metadata.Filepath);
		const char* filepath = metadata.Filepath.string().c_str();
		stbi_uc* imageData = nullptr;
		ImageFormat format = ImageFormat::Unspecified;

		if (stbi_is_hdr(filepath))
		{
			// TODO: add this capability
			Z_CORE_ASSERT(false, "Not currently supporting HDR texture formats")
			/*format = ;
			imageData = stbi_loadf();*/
		}
		else
		{
			format = ImageFormat::SRGB;
			imageData = stbi_load(filepath, &width, &height, &channels, channels);
		}

		Buffer buffer((void*)imageData, (uint64_t)(width * height * Image::BytesPerPixel(format)));
		Ref<Texture2D> newTexture;

		Texture2DSpecification spec{};
		spec.Format = format;
		spec.Width = width;
		spec.Height = height;
		// TODO: should these always be true?
		spec.GenerateMips = true;
		spec.KeepLocalData = true;

		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::OpenGL:
			{
				Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported");
				break;
			}
			case RendererAPI::API::DX12:
			{
				Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported");
				break;
			}
			case RendererAPI::API::Vulkan:
			{
				newTexture = Ref<VulkanTexture2D>::Create(spec, buffer);
				break;
			}
			default:
			{
				Z_CORE_ASSERT(false, "Invalid RendererAPI::API value");
				break;
			}
		}

		stbi_image_free(imageData);
	}

}

