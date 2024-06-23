#include "zpch.h"
#include "Application.h"
#include "Zahra/Core/Input.h"
#include "Zahra/Renderer/Renderer.h"

namespace Zahra
{
	Application* Application::s_Instance = nullptr;


	Application::Application()
	{
		Z_CORE_ASSERT(!s_Instance, "Application already exists");
		s_Instance = this;

		m_Window = Scope<Window>(Window::Create());
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

			// Update layers
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(dt);
			
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
		dispatcher.Dispatch<WindowCloseEvent>(Z_BIND_EVENT_FN(Application::OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);
			if (e.Handled) break;
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

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		
		return true;
	}


}