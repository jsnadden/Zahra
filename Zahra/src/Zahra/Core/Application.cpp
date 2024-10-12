#include "zpch.h"
#include "Application.h"

#include "Zahra/Core/Input.h"
#include "Zahra/Renderer/Renderer.h"
#include "Zahra/Utils/PlatformUtils.h"
#include "Zahra/Scripting/ScriptEngine.h"

namespace Zahra
{
	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		Z_CORE_ASSERT(!specification.WorkingDirectory.empty(), "Must specify a working directory")
		std::filesystem::current_path(specification.WorkingDirectory);
		Z_CORE_INFO("Current working directory: {0}", std::filesystem::current_path().string());

		Z_CORE_ASSERT(!s_Instance, "Application already exists");
		s_Instance = this;

		// TODO: maybe fill out more WindowProperties details before window creation?
		m_Window = Window::Create(WindowProperties(specification.Name));
		m_Window->SetEventCallback(Z_BIND_EVENT_FN(Application::OnEvent));
		m_Window->ReadConfig();

		// Initialise subsystems
		Renderer::Init();
		// TODO: ressurect
		//ScriptEngine::Init();

		m_ImGuiLayer = ImGuiLayer::Create();
		PushOverlay(m_ImGuiLayer);

	}
	
	Application::~Application()
	{
		// TODO: ressurect
		//ScriptEngine::Shutdown();
		Renderer::Shutdown();
	}

	void Application::Run()
	{
		Z_CORE_INFO("Start of run loop");

		while (m_Running)
		{
			// Compute frame time
			float frameStartTime = Time::GetTime();
			float frameTimeStep = frameStartTime - m_PreviousFrameStartTime; // actual delta time
			float dt = glm::min<float>(frameTimeStep, 0.0333f); // regularised for some numerical stability
			m_PreviousFrameStartTime = frameStartTime;

			m_Window->PollEvents();

			if (!m_Minimised)
			{
				Renderer::NewFrame();

				// Update layers
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(dt);

				// Render ImGui layers
				m_ImGuiLayer->Begin();
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
				m_ImGuiLayer->End();

				m_Window->PresentImage();
			}
			
		}

		Z_CORE_INFO("End of run loop");
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowClosedEvent>(Z_BIND_EVENT_FN(Application::OnWindowClosed));
		dispatcher.Dispatch<WindowResizedEvent>(Z_BIND_EVENT_FN(Application::OnWindowResized));
		dispatcher.Dispatch<WindowMinimisedEvent>(Z_BIND_EVENT_FN(Application::OnWindowMinimised));

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

	bool Application::OnWindowMinimised(WindowMinimisedEvent& e)
	{
		m_Minimised = e.Minimised();
		return false;
	}

}
