#pragma once

#include "Zahra/Core.h"
#include "Zahra/Events/Event.h"
#include "Zahra/Events/ApplicationEvent.h"
#include "Zahra/Window.h"
#include "Zahra/LayerStack.h"

namespace Zahra
{
	class ZAHRA_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;

		LayerStack m_LayerStack;
	};

	// To be defined by client app
	Application* CreateApplication();
}
