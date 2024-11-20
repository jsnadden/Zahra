#include "zpch.h"
#include "ShaderResourceManager.h"

#include "Platform/Vulkan/VulkanShaderResourceManager.h"
#include "Zahra/Renderer/Renderer.h"

namespace Zahra
{
	Ref<ShaderResourceManager> ShaderResourceManager::Create(const ShaderResourceManagerSpecification& specification)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
			case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
			case RendererAPI::API::Vulkan:	return Ref<VulkanShaderResourceManager>::Create(specification);
		}
		Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
		return nullptr;
	}
}
