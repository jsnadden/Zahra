#include "zpch.h"
#include "VertexArray.h"

#include "Zahra/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Zahra
{
	
    Ref<VertexArray> VertexArray::Create()
	{
        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:      Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
        case RendererAPI::API::OpenGL:    return Ref<OpenGLVertexArray>::Create();
        case RendererAPI::API::DX12:  Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
        case RendererAPI::API::Vulkan:    Z_CORE_ASSERT(false, "RendererAPI::API::Vulkan is not currently supported"); return nullptr;
        }
        Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
        return nullptr;
	}

}
