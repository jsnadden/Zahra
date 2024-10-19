#include "zpch.h"
#include "RendererAPI.h"

#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Zahra
{
	#if defined(Z_RENDERERAPI_VULKAN)
		RendererAPI::API RendererAPI::s_API = RendererAPI::API::Vulkan;
	#elif defined(Z_RENDERERAPI_DX12)
		RendererAPI::API RendererAPI::s_API = RendererAPI::API::DX12;
	#else
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::None;
	#endif


	RendererAPI* RendererAPI::Create()
	{
		switch (s_API)
		{
			case RendererAPI::API::None:	Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
			case RendererAPI::API::OpenGL:	Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
			case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
			case RendererAPI::API::Vulkan:	return new VulkanRendererAPI();
		}
		Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
		return nullptr;
	}
}
