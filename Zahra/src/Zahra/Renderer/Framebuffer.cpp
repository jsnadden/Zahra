#include "zpch.h"
#include "Framebuffer.h"

#include "Zahra/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace Zahra
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:      Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
        case RendererAPI::API::OpenGL:    return CreateRef<OpenGLFramebuffer>(spec);
        case RendererAPI::API::Direct3D:  Z_CORE_ASSERT(false, "RendererAPI::API::Direct3D is not currently supported"); return nullptr;
        case RendererAPI::API::Vulkan:    Z_CORE_ASSERT(false, "RendererAPI::API::Vulkan is not currently supported"); return nullptr;
        }
        Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
        return nullptr;
	}
}


