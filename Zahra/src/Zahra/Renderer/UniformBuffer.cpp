#include "zpch.h"
#include "UniformBuffer.h"

#include "Zahra/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"

namespace Zahra
{
	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
			case RendererAPI::API::OpenGL:	Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
			case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
			case RendererAPI::API::Vulkan:	return Ref<VulkanUniformBuffer>::Create(size);
		}
		Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
		return nullptr;
	}

	Ref<UniformBuffer> UniformBuffer::Create(const void* data, uint32_t size, uint32_t offset)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:	Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
		case RendererAPI::API::OpenGL:	Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
		case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
		case RendererAPI::API::Vulkan:	return Ref<VulkanUniformBuffer>::Create(data, size, offset);
		}
		Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
		return nullptr;
	}

	Ref<UniformBufferPerFrame> UniformBufferPerFrame::Create(uint32_t bufferSize, uint32_t framesInFlight)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:	Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
		case RendererAPI::API::OpenGL:	Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
		case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
		case RendererAPI::API::Vulkan:	return Ref<VulkanUniformBufferPerFrame>::Create(bufferSize, framesInFlight);
		}
		Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
		return nullptr;
	}
}
