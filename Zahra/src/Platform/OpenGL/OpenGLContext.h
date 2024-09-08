#pragma once

#include "Zahra/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Zahra
{
	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* handle);

		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_WindowHandle;

	};
}
