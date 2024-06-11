#include "zpch.h"

#include "Buffer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Renderer.h"

namespace Zahra
{
    VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::None:      Z_CORE_ASSERT(false, "RendererAPI::None is not currently supported"); return nullptr;
            case RendererAPI::OpenGL:    return new OpenGLVertexBuffer(vertices, size);
            case RendererAPI::Direct3D:  Z_CORE_ASSERT(false, "RendererAPI::Direct3D is not currently supported"); return nullptr;
            case RendererAPI::Vulkan:    Z_CORE_ASSERT(false, "RendererAPI::Vulkan is not currently supported"); return nullptr;
        }
        Z_CORE_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

    IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count)
    {
        switch (Renderer::GetAPI())
        {
        case RendererAPI::None:      Z_CORE_ASSERT(false, "RendererAPI::None is not currently supported"); return nullptr;
        case RendererAPI::OpenGL:    return new OpenGLIndexBuffer(indices, count);
        case RendererAPI::Direct3D:  Z_CORE_ASSERT(false, "RendererAPI::Direct3D is not currently supported"); return nullptr;
        case RendererAPI::Vulkan:    Z_CORE_ASSERT(false, "RendererAPI::Vulkan is not currently supported"); return nullptr;
        }
        Z_CORE_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }
}