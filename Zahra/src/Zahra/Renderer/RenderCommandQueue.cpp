#include "zpch.h"
#include "RenderCommandQueue.h"

#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Zahra
{
	#if defined(Z_RENDERERAPI_VULKAN)
	Scope<RendererAPI> RenderCommandQueue::s_RendererAPI = CreateScope<VulkanRendererAPI>();
	#endif
}
