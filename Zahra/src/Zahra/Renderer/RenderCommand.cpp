#include "zpch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Zahra
{
	// TODO: make this configurable externally (e.g. ApplicationSpecifications?)
	Scope<RendererAPI> RenderCommand::s_RendererAPI = CreateScope<VulkanRendererAPI>();
	//Scope<RendererAPI> RenderCommand::s_RendererAPI = CreateScope<OpenGLRendererAPI>();

}
