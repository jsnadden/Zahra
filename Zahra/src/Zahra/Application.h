#pragma once

#include "Zahra/Core.h"
#include "Zahra/Events/Event.h"
#include "Zahra/Events/ApplicationEvent.h"
#include "Zahra/Window.h"

namespace Zahra
{
	class ZAHRA_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};

	// To be defined by client app
	Application* CreateApplication();
}
