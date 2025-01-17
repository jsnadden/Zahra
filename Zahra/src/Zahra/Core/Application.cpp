#include "zpch.h"
#include "Application.h"

#include "Zahra/Core/Input.h"
#include "Zahra/Core/Memory.h"
#include "Zahra/Core/Timer.h"
#include "Zahra/Renderer/Renderer.h"
#include "Zahra/Scripting/ScriptEngine.h"
#include "Zahra/Utils/PlatformUtils.h"

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

		Renderer::SetConfig(m_Specification.RendererConfig);

		// TODO: other WindowProperties?
		m_Window = Window::Create(WindowProperties(specification.Name));
		m_Window->SetEventCallback(Z_BIND_EVENT_FN(Application::OnEvent));
		
		Renderer::Init();

		ScriptEngine::InitCore();
		// TODO: get args from project config
		ScriptEngine::InitApp("../Examples/Bud/Assets/Scripts/Binaries/Bud.dll", true);

		if (m_Specification.ImGuiConfig.Enabled)
		{
			m_ImGuiLayer = ImGuiLayer::GetOrCreate();
			PushOverlay(m_ImGuiLayer);
		}

		m_Window->ReadConfig();
	}
	
	Application::~Application()
	{
		// should clean up all content prior to shutting down core systems... right?
		m_LayerStack.PopAll();

		ScriptEngine::Shutdown();
		Renderer::Shutdown();
	}

	void Application::Run()
	{
		Z_CORE_INFO("Start of run loop");

		while (m_Running)
		{
			// Compute frame time
			float frameStartTime = Time::GetTime();
			//float dt = frameStartTime - m_PreviousFrameStartTime; // actual time step
			float dt = glm::min<float>(frameStartTime - m_PreviousFrameStartTime, 0.0333f); // regularised time step (for numerical stability)
			m_PreviousFrameStartTime = frameStartTime;

			FlushCommandQueue();

			m_Window->PollEvents();

			if (!m_Minimised)
			{
				Renderer::BeginFrame();

				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(dt);

				if (m_Specification.ImGuiConfig.Enabled)
				{
					m_ImGuiLayer->Begin();

					for (Layer* layer : m_LayerStack)
						layer->OnImGuiRender();

					m_ImGuiLayer->End();
				}

				Renderer::EndFrame();
				Renderer::Present();
			}
		}

		Z_CORE_INFO("End of run loop");

		m_Window->WriteConfig();
	}

	void Application::SubmitToMainThread(const std::function<void()>& command)
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadCommandQueueMutex);

		m_MainThreadCommandQueue.emplace_back(command);
	}

	void Application::FlushCommandQueue()
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadCommandQueueMutex);

		for (auto& command : m_MainThreadCommandQueue)
			command();

		m_MainThreadCommandQueue.clear();
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizedEvent>(Z_BIND_EVENT_FN(Application::OnWindowResized));
		dispatcher.Dispatch<WindowMinimisedEvent>(Z_BIND_EVENT_FN(Application::OnWindowMinimised));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.Handled) break;
			(*it)->OnEvent(e);
		}

		if (!e.Handled)
			dispatcher.Dispatch<WindowClosedEvent>(Z_BIND_EVENT_FN(Application::OnWindowClosed));
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
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

	bool Application::OnWindowMinimised(WindowMinimisedEvent& e)
	{
		m_Minimised = e.Minimised();
		return false;
	}

}
