#include "zpch.h"
#include "CommandBuffer.h"

#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Zahra/Renderer/Renderer.h"

namespace Zahra
{
	Ref<CommandBuffer> CommandBuffer::Create(const CommandBufferSpecification& specification)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:	Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
		case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
		case RendererAPI::API::Vulkan:	return Ref<VulkanCommandBuffer>::Create(specification);
		}
		Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
		return nullptr;
	}
}
