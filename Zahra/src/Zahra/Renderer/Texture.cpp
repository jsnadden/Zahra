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
        case RendererAPI::API::None:      Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
        case RendererAPI::API::OpenGL:    return std::make_shared<OpenGLTexture2D>(path);
        case RendererAPI::API::Direct3D:  Z_CORE_ASSERT(false, "RendererAPI::API::Direct3D is not currently supported"); return nullptr;
        case RendererAPI::API::Vulkan:    Z_CORE_ASSERT(false, "RendererAPI::API::Vulkan is not currently supported"); return nullptr;
        }
        Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
        return nullptr;
	}

}

