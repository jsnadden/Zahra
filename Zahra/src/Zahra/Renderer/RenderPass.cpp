#include "zpch.h"
#include "RenderPass.h"

#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Zahra/Renderer/Renderer.h"

namespace Zahra
{
	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& specification)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:	Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
		case RendererAPI::API::OpenGL:	Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
		case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
		case RendererAPI::API::Vulkan:	return Ref<VulkanRenderPass>::Create(specification);
		}
		Z_CORE_ASSERT(false, "Unknown RendererAPI::API");

		return nullptr;
	}
}