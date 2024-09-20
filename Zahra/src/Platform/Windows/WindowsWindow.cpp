#include "zpch.h"
#include "WindowsWindow.h"

#include "Zahra/Core/Input.h"
#include "Zahra/Core/Application.h"
#include "Zahra/Events/ApplicationEvent.h"
#include "Zahra/Events/MouseEvent.h"
#include "Zahra/Events/KeyEvent.h"
#include "Zahra/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLContext.h"
#include "Platform/Vulkan/VulkanContext.h"

#include <shobjidl.h>
#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

#include <fstream>

namespace Zahra
{
	static bool s_GLFWInitialised = false;
	
	static void GLFWErrorCallback(int error, const char* description)
	{
		Z_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Scope<Window> Window::Create(const WindowProperties& props)
	{
		return CreateScope<WindowsWindow>(props);
	}

	WindowsWindow::WindowsWindow(const WindowProperties& props)
	{
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		WriteConfig();
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProperties& props)
	{
		#pragma region Set initial window data

		m_WindowData.Title = props.Title;
		m_WindowData.Rectangle.Width = props.Width;
		m_WindowData.Rectangle.Height = props.Height;
		m_WindowData.RectangleCache = m_WindowData.Rectangle;

		#pragma endregion

		#pragma region Initialise GLFW

		if (!s_GLFWInitialised)
		{
			s_GLFWInitialised = glfwInit();
			Z_CORE_ASSERT(s_GLFWInitialised, "GLFW failed to initialise");

			glfwSetErrorCallback(GLFWErrorCallback);
		}

		#pragma endregion

		#pragma region Initialise Windows COM library (used for open/save dialogs e.g.)

		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		Z_CORE_ASSERT(SUCCEEDED(hr), "Windows COM library failed to initialise.");

		#pragma endregion

		#pragma region Initialise renderer API debugging
		#if defined(Z_DEBUG)

		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::OpenGL:
			{
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
				break;
			}
			case RendererAPI::API::Vulkan:
			{
				glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

				// TODO: temporary
				glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
				break;
			}
			default: break;
		}

		#endif
		#pragma endregion

		#pragma region Construct window based on WindowData

		// TODO: custom titlebar
		//glfwWindowHint(GLFW_TITLEBAR, false);		

		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_WindowData.Title.c_str(), nullptr, nullptr);
		Z_CORE_INFO("Window successfully created: {0}", props.Title);
		
		glfwSetWindowPos(m_Window, m_WindowData.Rectangle.XPosition, m_WindowData.Rectangle.YPosition);
		glfwSetWindowUserPointer(m_Window, &m_WindowData);

		#pragma endregion

		#pragma region Initialise renderer context

		m_Context = RendererContext::Create(m_Window);
		m_Context->Init();

		#pragma endregion

		#pragma region Set win32 event callbacks
		
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

		glfwSetWindowIconifyCallback(m_Window,
			[](GLFWwindow* window, int iconified)
			{
				Win32WindowData& data = *(Win32WindowData*)glfwGetWindowUserPointer(window);

				if (iconified == GLFW_TRUE)
				{
					data.RectangleCache = data.Rectangle;
					data.State = WindowState::Minimised;
				}
				else
					data.State = WindowState::Default;

				WindowMinimisedEvent event(iconified);
				data.EventCallback(event);
			}
		);

		glfwSetWindowMaximizeCallback(m_Window,
			[](GLFWwindow* window, int maximised)
			{
				Win32WindowData& data = *(Win32WindowData*)glfwGetWindowUserPointer(window);

				if (maximised == GLFW_TRUE)
				{
					data.RectangleCache = data.Rectangle;
					data.State = WindowState::Maximised;
				}
				else
					data.State = WindowState::Default;

				WindowMaximisedEvent event(maximised);
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

}

	void WindowsWindow::Shutdown()
	{
		m_Context->Shutdown();
		glfwDestroyWindow(m_Window);
		glfwTerminate();
		CoUninitialize(); // Shutdown Windows COM library
	}

	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();

		m_Context->SwapBuffers();
	}

	void WindowsWindow::Resize(uint32_t width, uint32_t height)
	{
		if (m_WindowData.Fullscreen) return;
		glfwSetWindowSize(m_Window, (int)width, (int)height);
	}

	std::pair<uint32_t, uint32_t> WindowsWindow::GetPosition() const
	{
		uint32_t x = m_WindowData.Rectangle.XPosition;
		uint32_t y = m_WindowData.Rectangle.YPosition;
		return std::make_pair(x,y);
	}

	void WindowsWindow::Move(uint32_t x, uint32_t y)
	{
		if (m_WindowData.Fullscreen) return;
		glfwSetWindowPos(m_Window, (int)x, (int)y);
	}

	void WindowsWindow::SetState(WindowState state)
	{
		if (state == m_WindowData.State || m_WindowData.Fullscreen) return;

		switch (state)
		{
			case WindowState::Default:
			{
				glfwRestoreWindow(m_Window);
				break;
			}
			case WindowState::Minimised:
			{
				m_WindowData.RectangleCache = m_WindowData.Rectangle;
				glfwIconifyWindow(m_Window);
				break;
			}
			case WindowState::Maximised:
			{
				m_WindowData.RectangleCache = m_WindowData.Rectangle;
				glfwMaximizeWindow(m_Window);
				break;
			}
			default: break;
		}
	}

	bool WindowsWindow::IsFullscreen() const
	{
		return m_WindowData.Fullscreen;
	}

	void WindowsWindow::SetFullscreen(bool enabled)
	{
		if (m_WindowData.Fullscreen == enabled) return;

		if (m_WindowData.Fullscreen)
		{
			glfwSetWindowMonitor(m_Window,nullptr,
				m_WindowData.RectangleCache.XPosition, m_WindowData.RectangleCache.YPosition,
				m_WindowData.RectangleCache.Width, m_WindowData.RectangleCache.Height, 0);
		}
		else
		{
			if (m_WindowData.State == WindowState::Default) m_WindowData.RectangleCache = m_WindowData.Rectangle;
			
			// TODO: find a good way to choose the "current monitor" (Win32 api has this, but not glfw...)
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
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
		/*if (enabled)
		{
			glfwSwapInterval(1);
		}
		else
		{
			glfwSwapInterval(0);
		}*/

		m_WindowData.VSync = enabled;
	}	
	
	void WindowsWindow::WriteConfig()
	{
		Z_CORE_TRACE("Saving configuration for window '{0}'", m_WindowData.Title);

		WindowState currentState = GetState();
		SetState(WindowState::Default);

		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "WindowRectangle";
		out << YAML::BeginMap;		
		{
			WindowRectangle rect;
			if (m_WindowData.Fullscreen)
				rect = m_WindowData.RectangleCache;
			else
				rect = m_WindowData.Rectangle;

			out << YAML::Key << "Width";
			out << YAML::Value << (rect.Width);

			out << YAML::Key << "Height";
			out << YAML::Value << (rect.Height);

			out << YAML::Key << "XPosition";
			out << YAML::Value << (rect.XPosition);

			out << YAML::Key << "YPosition";
			out << YAML::Value << (rect.YPosition);
		}
		out << YAML::EndMap;

		out << YAML::Key << "WindowState";
		out << YAML::Value << currentState;

		out << YAML::Key << "Fullscreen";
		out << YAML::Value << m_WindowData.Fullscreen;

		out << YAML::Key << "VSync";
		out << YAML::Value << m_WindowData.VSync;

		out << YAML::EndMap;

		std::filesystem::path configDir = Application::Get().GetSpecification().WorkingDirectory / "Config";
		if (!std::filesystem::exists(configDir)) std::filesystem::create_directories(configDir);

		std::filesystem::path configFile = configDir / ("window_config_" + m_WindowData.Title + ".yml");
		std::ofstream fout(configFile.c_str());
		fout << out.c_str();

	}

	void WindowsWindow::ReadConfig()
	{
		std::filesystem::path configDir = Application::Get().GetSpecification().WorkingDirectory / "Config";
		std::filesystem::path configFile = configDir / ("window_config_" + m_WindowData.Title + ".yml");
		if (!std::filesystem::exists(configFile)) return;

		Z_CORE_TRACE("Loading configuration for window '{0}'", m_WindowData.Title);

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(configFile.string());
		}
		catch (const YAML::ParserException& ex)
		{
			Z_CORE_ERROR("Failed to load window configuration file '{0}'\n     {1}", configFile.filename().string(), ex.what());
		}

		if (auto rectNode = data["WindowRectangle"])
		{
			uint32_t width = rectNode["Width"].as<uint32_t>();
			uint32_t height = rectNode["Height"].as<uint32_t>();
			Resize(width, height);

			uint32_t x = rectNode["XPosition"].as<uint32_t>();
			uint32_t y = rectNode["YPosition"].as<uint32_t>();
			Move(x, y);
		}

		if (auto stateNode = data["WindowState"])
		{
			WindowState state = (WindowState)stateNode.as<int>();
			if (state != WindowState::Minimised) SetState(state);
		}

		if (auto fullscreenNode = data["Fullscreen"])  SetFullscreen(fullscreenNode.as<bool>());

		if (auto vsyncNode = data["VSync"]) SetVSync(vsyncNode.as<bool>());

	}


}


