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
		#pragma region Set initial window data

		m_WindowData.Title = props.Title;
		m_WindowData.Rectangle.Width = props.Width;
		m_WindowData.Rectangle.Height = props.Height;
		Z_CORE_INFO("Creating window {0} ({1}x{2})", props.Title, props.Width, props.Height);

		#pragma endregion

		#pragma region Initialise GLFW

		if (!s_GLFWInitialised)
		{
			Z_CORE_ASSERT(glfwInit(), "GLFW failed to initialise");

			glfwSetErrorCallback(GLFWErrorCallback);

			s_GLFWInitialised = true;
		}

		#pragma endregion

		#pragma region Initialise Windows COM library (used for open/save dialogs e.g.)

		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		Z_CORE_ASSERT(SUCCEEDED(hr), "Windows COM library failed to initialise.");

		#pragma endregion

		#pragma region Initialise renderer API debugging
		#if defined(Z_DEBUG)

		if (Renderer::GetAPI() == RendererAPI::API::OpenGL) glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
		// TODO: if (Renderer::GetAPI() == RendererAPI::API::Direct3D) DO SOMETHING;
		// TODO: if (Renderer::GetAPI() == RendererAPI::API::Vulkan) DO SOMETHING;

		#endif		
		#pragma endregion

		#pragma region Construct window based on WindowData

		glfwWindowHint(GLFW_TITLEBAR, m_WindowData.ShowTitleBar);		
		GLFWmonitor* mainMonitor = glfwGetPrimaryMonitor();
		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_WindowData.Title.c_str(),
			m_WindowData.Fullscreen ? mainMonitor : nullptr, nullptr);
		glfwSetWindowPos(m_Window, m_WindowData.Rectangle.XPosition, m_WindowData.Rectangle.YPosition);

		#pragma endregion

		#pragma region Initialise renderer context

		m_Context = CreateScope<OpenGLContext>(m_Window);
		m_Context->Init();

		#pragma endregion

		#pragma region Set win32 window data

		glfwSetWindowUserPointer(m_Window, &m_WindowData);

		glfwSetWindowSizeCallback(m_Window,
			[](GLFWwindow* window, int width, int height)
			{
				Win32WindowData& data = *(Win32WindowData*)glfwGetWindowUserPointer(window);
				data.Rectangle.Width = width;
				data.Rectangle.Height = height;

				WindowResizedEvent event(width, height);
				data.EventCallback(event);
			}
		);

		glfwSetWindowPosCallback(m_Window,
			[](GLFWwindow* window, int x, int y)
			{
				Win32WindowData& data = *(Win32WindowData*)glfwGetWindowUserPointer(window);
				data.Rectangle.XPosition = x;
				data.Rectangle.YPosition = y;

				WindowMovedEvent event((float)x, (float)y);
				data.EventCallback(event);
			}
		);

		glfwSetWindowCloseCallback(m_Window,
			[](GLFWwindow* window)
			{
				Win32WindowData& data = *(Win32WindowData*)glfwGetWindowUserPointer(window);

				WindowClosedEvent event;
				data.EventCallback(event);
			}
		);

		glfwSetKeyCallback(m_Window,
			[](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				Win32WindowData& data = *(Win32WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(static_cast<KeyCode>(key), false);
					data.EventCallback(event);
					break;
				}					

				case GLFW_REPEAT:
				{
					KeyPressedEvent event(static_cast<KeyCode>(key), true);
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
				Win32WindowData& data = *(Win32WindowData*)glfwGetWindowUserPointer(window);

				KeyTypedEvent event(static_cast<KeyCode>(keycode));
				data.EventCallback(event);
			}
		);

		glfwSetMouseButtonCallback(m_Window,
			[](GLFWwindow* window, int button, int action, int mods)
			{
				Win32WindowData& data = *(Win32WindowData*)glfwGetWindowUserPointer(window);

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
				Win32WindowData& data = *(Win32WindowData*)glfwGetWindowUserPointer(window);

				MouseScrolledEvent event((float)xOffset, (float)yOffset);
				data.EventCallback(event);
			}
		);

		glfwSetCursorPosCallback(m_Window,
			[](GLFWwindow* window, double x, double y)
			{
				Win32WindowData& data = *(Win32WindowData*)glfwGetWindowUserPointer(window);

				MouseMovedEvent event((float)x, (float)y);
				data.EventCallback(event);
			}
		);

		#pragma endregion
	
		ReadConfig();
}

	void WindowsWindow::Shutdown()
	{
		WriteConfig();

		glfwDestroyWindow(m_Window);

		CoUninitialize(); // Shutdown Windows COM library
	}

	void WindowsWindow::ReadConfig()
	{
		// TODO:
		// - get config directory (from AppSpec, or a core config file)
		// - check if window_config.yml exists, early out if not
		// - if it does exist, read it in
		// - parse (key,value)s, and call the appropriate methods (ToggleFullscreen, SetVSync, etc.)
		// (another TODO: add methods to resize/reposition window)
	}

	void WindowsWindow::WriteConfig()
	{
		// TODO:
		// - get config directory (from AppSpec, or a core config file)
		// - create window_config.yml if it doesn't already exist
		// - write (key, value)s (most we can just get from glfw, but also e.g. m_FullscreenRectangleCache)
	}

	void WindowsWindow::OnUpdate()
	{
		Z_PROFILE_FUNCTION();

		glfwPollEvents();

		m_Context->SwapBuffers();
	}

	std::pair<uint32_t, uint32_t> WindowsWindow::GetPosition() const
	{
		uint32_t x = m_WindowData.Rectangle.XPosition;
		uint32_t y = m_WindowData.Rectangle.YPosition;
		return std::make_pair(x,y);
	}

	bool WindowsWindow::IsFullscreen() const
	{
		return m_WindowData.Fullscreen;
	}

	void WindowsWindow::ToggleFullscreen()
	{
		if (m_WindowData.Fullscreen)
		{
			glfwSetWindowMonitor(m_Window,nullptr,
				m_FullscreenRectangleCache.XPosition, m_FullscreenRectangleCache.YPosition,
				m_FullscreenRectangleCache.Width, m_FullscreenRectangleCache.Height, 0);
		}
		else
		{
			m_FullscreenRectangleCache = m_WindowData.Rectangle;
			
			// TODO: find a good way to choose the "current monitor" (Win32 api has this, but not glfw...)
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			Z_CORE_INFO("Going fullscreen: {}x{}", mode->width, mode->height);
			// TODO: get fullscreen resolution from settings/config file rather than the monitor itself
			glfwSetWindowMonitor(m_Window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		}

		m_WindowData.Fullscreen = !m_WindowData.Fullscreen;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_WindowData.VSync;
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		// TODO: make this actually work...
		if (enabled)
		{
			glfwSwapInterval(1);
		}
		else
		{
			glfwSwapInterval(0);
		}

		m_WindowData.VSync = enabled;
	}	
	
}


