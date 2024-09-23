#include "zpch.h"
#include "Texture.h"

#include "Zahra/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Zahra
{

	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{

        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:    Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
        case RendererAPI::API::OpenGL:  return Ref<OpenGLTexture2D>::Create(path);
        case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
        case RendererAPI::API::Vulkan:	Z_CORE_ASSERT(false, "RendererAPI::API::Vulkan is not currently supported"); return nullptr;
        }
        Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
        return nullptr;
	}

    Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
    {
        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:    Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
        case RendererAPI::API::OpenGL:  return Ref<OpenGLTexture2D>::Create(width, height);
        case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
        case RendererAPI::API::Vulkan:  Z_CORE_ASSERT(false, "RendererAPI::API::Vulkan is not currently supported"); return nullptr;
        }
        Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
        return nullptr;
    }

}

