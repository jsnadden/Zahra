#include "zpch.h"
#include "RendererAPI.h"

namespace Zahra
{
	#if defined(Z_RENDERERAPI_VULKAN)
		RendererAPI::API RendererAPI::s_API = RendererAPI::API::Vulkan;
	#elif defined(Z_RENDERERAPI_DX12)
		RendererAPI::API RendererAPI::s_API = RendererAPI::API::DX12;
	#else
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::None;
	#endif
}
