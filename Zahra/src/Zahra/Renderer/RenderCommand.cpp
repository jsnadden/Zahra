#include "zpch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Zahra
{
	// hard coding opengl, until we implement other apis
	RendererAPI* RenderCommand::s_rendererAPI = new OpenGLRendererAPI();

}