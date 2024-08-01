#include "zpch.h"
#include "WindowsWindow.h"

#include "Zahra/Core/Input.h"
#include "Zahra/Events/ApplicationEvent.h"
#include "Zahra/Events/MouseEvent.h"
#include "Zahra/Events/KeyEvent.h"
#include "Zahra/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLContext.h"

#include <shobjidl.h>

namespace Zahra
{
	static bool s_GLFWInitialised = false;

	static void GLFWErrorCallback(int error, const char* description)
	{
		Z_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Scope<Window> Window::Create(const WindowProperties& props)
	{
		Z_PROFILE_FUNCTION();

		return CreateScope<WindowsWindow>(props);
	}

	WindowsWindow::WindowsWindow(const WindowProperties& props)
	{
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProperties& props)
	{
		Z_PROFILE_FUNCTION();

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		Z_CORE_INFO("Creating window {0} ({1}x{2})", props.Title, props.Width, props.Height);


		if (!s_GLFWInitialised)
		{
			Z_PROFILE_SCOPE("glfwInit");
			int success = glfwInit();
			Z_CORE_ASSERT(success, "Failed to initialise GLFW");

			glfwSetErrorCallback(GLFWErrorCallback);

			s_GLFWInitialised = true;
		}

		// Initialise Windows COM library (used for open/save dialogs e.g.)
		{
			HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			Z_CORE_ASSERT(SUCCEEDED(hr), "Windows COM library failed to initialise.");
		}

		
		{
			{
				#if defined(Z_DEBUG)
					if (Renderer::GetAPI() == RendererAPI::API::OpenGL) glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
					// TODO: if (Renderer::GetAPI() == RendererAPI::API::Direct3D) DO SOMETHING;
					// TODO: if (Renderer::GetAPI() == RendererAPI::API::Vulkan) DO SOMETHING;
				#endif
			}

			
			// TODO: activate this if I end up making a custom title bar: glfwWindowHint(GLFW_TITLEBAR, false);

			m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);

		}

		m_Context = CreateScope<OpenGLContext>(m_Window);

		m_Context->Init();		

		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		// Set GLFW callbacks

		glfwSetWindowSizeCallback(m_Window,
			[](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.Width = width;
				data.Height = height;

				WindowResizedEvent event(width, height);
				//Z_CORE_WARN("Window resized: {0}, {1}", width, height);
				data.EventCallback(event);
			}
		);

		glfwSetWindowCloseCallback(m_Window,
			[](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				WindowClosedEvent event;
				data.EventCallback(event);
			}
		);

		glfwSetKeyCallback(m_Window,
			[](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(static_cast<KeyCode>(key), 0);
					data.EventCallback(event);
					break;
				}					

				case GLFW_REPEAT:
				{
					KeyPressedEvent event(static_cast<KeyCode>(key), 1);
					data.EventCallback(event);
					break;
				}

				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(static_cast<KeyCode>(key));
					data.EventCallback(event);
					break;
				}
				}
			}
		);

		glfwSetCharCallback(m_Window,
			[](GLFWwindow* window, unsigned int keycode)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				KeyTypedEvent event(static_cast<KeyCode>(keycode));
				data.EventCallback(event);
			}
		);

		glfwSetMouseButtonCallback(m_Window,
			[](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(static_cast<MouseCode>(button));
					data.EventCallback(event);
					break;
				}

				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(static_cast<MouseCode>(button));
					data.EventCallback(event);
					break;
				}
				}
			}
		);

		glfwSetScrollCallback(m_Window,
			[](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseScrolledEvent event((float)xOffset, (float)yOffset);
				data.EventCallback(event);
			}
		);

		glfwSetCursorPosCallback(m_Window,
			[](GLFWwindow* window, double x, double y)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseMovedEvent event((float)x, (float)y);
				data.EventCallback(event);
			}
		);

	}

	void WindowsWindow::Shutdown()
	{
		Z_PROFILE_FUNCTION();

		glfwDestroyWindow(m_Window);

		CoUninitialize();
	}

	void WindowsWindow::OnUpdate()
	{
		Z_PROFILE_FUNCTION();

		glfwPollEvents();

		m_Context->SwapBuffers();
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		Z_PROFILE_FUNCTION();

		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}
	
}


