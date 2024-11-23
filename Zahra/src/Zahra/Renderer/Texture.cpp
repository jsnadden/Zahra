#include "zpch.h"
#include "Texture.h"

#include "Zahra/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanTexture.h"

namespace Zahra
{

	Ref<Texture2D> Texture2D::Create(const Texture2DSpecification& specification)
	{

        switch (Renderer::GetAPI())
        {
			case RendererAPI::API::None:    Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
			case RendererAPI::API::OpenGL:  Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
			case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
			case RendererAPI::API::Vulkan:	return Ref<VulkanTexture2D>::Create(specification);
		}

        Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
        return nullptr;
	}

    Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
    {
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
			case RendererAPI::API::OpenGL:  Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
			case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
			case RendererAPI::API::Vulkan:	return Ref<VulkanTexture2D>::Create(width, height);
		}

        Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
        return nullptr;
    }

}

