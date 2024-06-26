#include "zpch.h"
#include "Buffer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Renderer.h"

namespace Zahra
{
    Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
    {
        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:      Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
        case RendererAPI::API::OpenGL:    return CreateRef<OpenGLVertexBuffer>(size);
        case RendererAPI::API::Direct3D:  Z_CORE_ASSERT(false, "RendererAPI::API::Direct3D is not currently supported"); return nullptr;
        case RendererAPI::API::Vulkan:    Z_CORE_ASSERT(false, "RendererAPI::API::Vulkan is not currently supported"); return nullptr;
        }
        Z_CORE_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }
    Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t count)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:      Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
            case RendererAPI::API::OpenGL:    return CreateRef<OpenGLVertexBuffer>(vertices, count);
            case RendererAPI::API::Direct3D:  Z_CORE_ASSERT(false, "RendererAPI::API::Direct3D is not currently supported"); return nullptr;
            case RendererAPI::API::Vulkan:    Z_CORE_ASSERT(false, "RendererAPI::API::Vulkan is not currently supported"); return nullptr;
        }
        Z_CORE_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

    Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
    {
        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:      Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
        case RendererAPI::API::OpenGL:    return CreateRef<OpenGLIndexBuffer>(indices, count);
        case RendererAPI::API::Direct3D:  Z_CORE_ASSERT(false, "RendererAPI::API::Direct3D is not currently supported"); return nullptr;
        case RendererAPI::API::Vulkan:    Z_CORE_ASSERT(false, "RendererAPI::API::Vulkan is not currently supported"); return nullptr;
        }
        Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
        return nullptr;
    }
}