#include "zpch.h"
#include "Application.h"
#include "Zahra/Core/Input.h"
#include "Zahra/Renderer/Renderer.h"

namespace Zahra
{
	Application* Application::s_Instance = nullptr;


	Application::Application(const std::string& name, ApplicationCommandLineArgs args)
		: m_CommandLineArgs(args)
	{
		Z_PROFILE_FUNCTION();

		Z_CORE_INFO("Current working directory {0}", std::filesystem::current_path().string());

		Z_CORE_ASSERT(!s_Instance, "Application already exists");
		s_Instance = this;

		m_Window = Window::Create(WindowProperties(name));

		m_Window->SetEventCallback(Z_BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer;
		PushOverlay(m_ImGuiLayer);

	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		while (m_Running)
		{
			// Compute frame time
			float ThisFrameTime = (float)glfwGetTime(); // TODO MAKE THIS PLATFORM-INDEPENDENT!!
			float dt = ThisFrameTime - m_PreviousFrameTime;
			m_PreviousFrameTime = ThisFrameTime;

			if (!m_Minimised)
			{
				// Update layers
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(dt);
			}

			// Render ImGui layers
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			// Update window context
			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowClosedEvent>(Z_BIND_EVENT_FN(Application::OnWindowClosed));
		dispatcher.Dispatch<WindowResizedEvent>(Z_BIND_EVENT_FN(Application::OnWindowResized));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.Handled) break;
			(*it)->OnEvent(e);
		}

	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::Exit()
	{
		m_Running = false;
	}

	bool Application::OnWindowClosed(WindowClosedEvent& e)
	{
		Exit();
		return true;
	}

	bool Application::OnWindowResized(WindowResizedEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimised = true;
			return false;
		}
		
		m_Minimised = false;

		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}


}
