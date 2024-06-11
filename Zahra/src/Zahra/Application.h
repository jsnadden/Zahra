#pragma once

#include "Zahra/Base.h"
#include "Zahra/Events/Event.h"
#include "Zahra/Events/ApplicationEvent.h"
#include "Zahra/Window.h"
#include "Zahra/LayerStack.h"

#include "Zahra/ImGui/ImGuiLayer.h"

// temporarily including all of this, while we're rendering directly in the run loop
#include "Zahra/Renderer/Buffer.h"
#include "Zahra/Renderer/Shader.h"
#include "Zahra/Renderer/VertexArray.h"
#include  "Zahra/Renderer/Renderer.h"

namespace Zahra
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline static Application& Get() { return *s_Instance; }

		inline Window& GetWindow() { return *m_Window; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;

		bool m_Running = true;

		LayerStack m_LayerStack;

		std::shared_ptr<Shader> m_Shader;
		std::shared_ptr<VertexArray> m_VertexArray;

		static Application* s_Instance;
	};

	// To be defined by client app
	Application* CreateApplication();
}
