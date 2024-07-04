#pragma once

#include "Zahra/Core/Base.h"
#include "Zahra/Events/Event.h"
#include "Zahra/Events/ApplicationEvent.h"
#include "Zahra/Core/Window.h"
#include "Zahra/Core/LayerStack.h"

#include "Zahra/ImGui/ImGuiLayer.h"


///////////////////////////////////////////////////
// TODO: We're using a GLFW include to compute
// timesteps for now, but those calculations
// should eventually become platform-independent
#include <../vendor/GLFW/include/GLFW/glfw3.h>
///////////////////////////////////////////////////


namespace Zahra
{
	class Application
	{
	public:
		Application(const std::string& name = "Zahra");
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		static Application& Get() { return *s_Instance; }

		Window& GetWindow() { return *m_Window; }

		void Exit();

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

	private:
		bool OnWindowClosed(WindowClosedEvent& e);
		bool OnWindowResized(WindowResizedEvent& e);

		static Application* s_Instance;

		Scope<Window> m_Window;

		ImGuiLayer* m_ImGuiLayer;

		bool m_Running = true;
		bool m_Minimised = false;

		float m_PreviousFrameTime;

		LayerStack m_LayerStack;
	};

	// To be defined by client app
	Application* CreateApplication();
}
