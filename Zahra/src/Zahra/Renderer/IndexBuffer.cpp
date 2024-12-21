#include "zpch.h"
#include "IndexBuffer.h"

#include "Platform/Vulkan/VulkanIndexBuffer.h"
#include "Zahra/Renderer/Renderer.h"

namespace Zahra
{
	Ref<IndexBuffer> IndexBuffer::Create(const uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:	Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
		case RendererAPI::API::OpenGL:	Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
		case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
		case RendererAPI::API::Vulkan:	return Ref<VulkanIndexBuffer>::Create(indices, count);
		}
		Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
		return nullptr;
	}
}
