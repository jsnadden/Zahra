#include "zpch.h"
#include "VertexBuffer.h"

#include "Platform/Vulkan/VulkanVertexBuffer.h"
#include "Zahra/Renderer/Renderer.h"

namespace Zahra
{
    Ref<VertexBuffer> VertexBuffer::Create(uint64_t size)
    {
        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:	Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
        case RendererAPI::API::OpenGL:	Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
        case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
        case RendererAPI::API::Vulkan:	return Ref<VulkanVertexBuffer>::Create(size);
        }
        Z_CORE_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

    Ref<VertexBuffer> VertexBuffer::Create(const void* data, uint64_t size)
    {
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:	Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
		case RendererAPI::API::OpenGL:	Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
		case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
		case RendererAPI::API::Vulkan:	return Ref<VulkanVertexBuffer>::Create(data, size);
		}
		Z_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
    }

}
