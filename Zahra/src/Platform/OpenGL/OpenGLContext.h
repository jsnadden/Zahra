#pragma once

#include "Zahra/Renderer/RendererContext.h"

struct GLFWwindow;

namespace Zahra
{
	class OpenGLContext : public RendererContext
	{
	public:
		OpenGLContext(GLFWwindow* handle);

		virtual void Init() override;
		virtual void Shutdown() override;
		virtual void PresentImage() override;

	private:
		GLFWwindow* m_WindowHandle;

	};
}
