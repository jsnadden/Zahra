#include "zpch.h"
#include "Texture.h"

#include "Zahra/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanTexture.h"

namespace Zahra
{

	Ref<Texture2D> Texture2D::CreateFromFile(const Texture2DSpecification& specification, std::filesystem::path filepath)
	{
		switch (Renderer::GetAPI())
        {
			case RendererAPI::API::None:    Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
			case RendererAPI::API::OpenGL:  Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
			case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
			case RendererAPI::API::Vulkan:	return Ref<VulkanTexture2D>::Create(specification, filepath);
		}

        Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
        return nullptr;
	}

	Ref<Texture2D> Texture2D::CreateFromImage2D(const Ref<Image2D>& image)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
		case RendererAPI::API::OpenGL:  Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
		case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
		case RendererAPI::API::Vulkan:	return Ref<VulkanTexture2D>::Create(image);
		}

		Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
		return nullptr;
	}

    Ref<Texture2D> Texture2D::CreateFlatColourTexture(const Texture2DSpecification& specification, uint32_t colour)
    {
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
			case RendererAPI::API::OpenGL:  Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
			case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
			case RendererAPI::API::Vulkan:	return Ref<VulkanTexture2D>::Create(specification, colour);
		}

        Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
        return nullptr;
    }

}

