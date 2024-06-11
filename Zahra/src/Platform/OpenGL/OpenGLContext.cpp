#include "zpch.h"

#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Zahra
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_windowHandle(windowHandle)
	{
		Z_CORE_ASSERT(windowHandle, "Window handle is NULL")
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_windowHandle);

		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		Z_CORE_ASSERT(status, "Failed to initialise Glad");

		Z_CORE_INFO("OpenGL initialised");
		Z_CORE_INFO("       Version: {0}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
		Z_CORE_INFO("      Renderer: {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_windowHandle);
	}
}
