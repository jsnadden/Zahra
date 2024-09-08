#include "zpch.h"
#include "RendererAPI.h"

namespace Zahra
{
	// TODO: make this configurable externally (e.g. ApplicationSpecifications?)
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::Vulkan;

}
