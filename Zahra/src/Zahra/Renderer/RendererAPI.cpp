#include "zpch.h"
#include "RendererAPI.h"

namespace Zahra
{
	// hard coding opengl, until we implement other apis
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

}