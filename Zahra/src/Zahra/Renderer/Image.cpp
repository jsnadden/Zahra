#include "zpch.h"
#include "Image.h"

#include "Platform/Vulkan/VulkanImage.h"
#include "Zahra/Renderer/Renderer.h"

namespace Zahra
{
	Ref<Image> Image::Create(uint32_t width, uint32_t height, ImageFormat format, ImageUsage usage)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:	Z_CORE_ASSERT(false, "RendererAPI::API::None is not currently supported"); return nullptr;
		case RendererAPI::API::OpenGL:	Z_CORE_ASSERT(false, "RendererAPI::API::OpenGL is no longer supported"); return nullptr;
		case RendererAPI::API::DX12:	Z_CORE_ASSERT(false, "RendererAPI::API::DX12 is not currently supported"); return nullptr;
		case RendererAPI::API::Vulkan:	return Ref<VulkanImage>::Create(width, height, format, usage);
		}
		Z_CORE_ASSERT(false, "Unknown RendererAPI::API");
		return nullptr;
	}
}
