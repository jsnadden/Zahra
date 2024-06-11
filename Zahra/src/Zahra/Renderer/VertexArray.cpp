#include "zpch.h"
#include "VertexArray.h"

#include "Zahra/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Zahra
{
	
    VertexArray* VertexArray::Create()
	{
        switch (Renderer::GetAPI())
        {
        case RendererAPI::None:      Z_CORE_ASSERT(false, "RendererAPI::None is not currently supported"); return nullptr;
        case RendererAPI::OpenGL:    return new OpenGLVertexArray();
        case RendererAPI::Direct3D:  Z_CORE_ASSERT(false, "RendererAPI::Direct3D is not currently supported"); return nullptr;
        case RendererAPI::Vulkan:    Z_CORE_ASSERT(false, "RendererAPI::Vulkan is not currently supported"); return nullptr;
        }
        Z_CORE_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
	}

}