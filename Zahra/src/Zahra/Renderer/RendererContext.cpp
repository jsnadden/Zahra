#include "zpch.h"
#include "RendererContext.h"

#include "Zahra/Renderer/RendererAPI.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/OpenGL/OpenGLContext.h"

namespace Zahra
{
	Ref<RendererContext> RendererContext::Create(GLFWwindow* windowHandle)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:	Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
		case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
		case RendererAPI::API::OpenGL:	return CreateRef<OpenGLContext>(windowHandle);
		case RendererAPI::API::Vulkan:	return Ref<VulkanContext>(VulkanContext::Get(windowHandle));
		}
		Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
		return nullptr;
	}
}
