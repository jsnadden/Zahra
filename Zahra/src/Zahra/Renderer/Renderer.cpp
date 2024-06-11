#include "zpch.h"
#include "Renderer.h"

namespace Zahra
{
	// just hardcoded opengl for now, until another api is implemented
	RendererAPI Renderer::s_RendererAPI = RendererAPI::OpenGL;
}