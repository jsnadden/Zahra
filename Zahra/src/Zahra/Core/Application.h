#pragma once

#include "Zahra/Core/AppRequirements.h"
#include "Zahra/Core/Defines.h"
#include "Zahra/Core/LayerStack.h"
#include "Zahra/Core/Window.h"
#include "Zahra/Events/ApplicationEvent.h"
#include "Zahra/Events/Event.h"
#include "Zahra/ImGui/ImGuiLayer.h"
#include "Zahra/Renderer/RendererConfig.h"

namespace Zahra
{
	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			Z_CORE_ASSERT(index < Count);
			return Args[index];
		}
	};

	struct ApplicationVersion
	{
		uint16_t Major = 0;
		uint16_t Minor = 0;
		uint16_t Patch = 0;

		ApplicationVersion() = default;
		ApplicationVersion(int major, int minor, int patch)
			: Major(major), Minor(minor), Patch(patch) {}
	};

	struct ApplicationSpecification
	{
		std::string Name = "Zahra_App";
		ApplicationVersion Version;

		std::filesystem::path WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;

		RendererConfig RendererConfig;
		GPURequirements GPURequirements;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& spec);
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		static inline Application& Get() { return *s_Instance; }

		const ApplicationSpecification& GetSpecification() const { return m_Specification; }

		inline Window& GetWindow() { return *m_Window; }

		void Exit();

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

	private:
		ApplicationSpecification m_Specification;

		static Application* s_Instance;

		Scope<Window> m_Window;

		ImGuiLayer* m_ImGuiLayer;

		bool m_Running = true;
		bool m_Minimised = false;

		float m_PreviousFrameStartTime;

		LayerStack m_LayerStack;

		bool OnWindowClosed(WindowClosedEvent& e);
		bool OnWindowResized(WindowResizedEvent& e);
		bool OnWindowMinimised(WindowMinimisedEvent& e);


	};

	// To be defined by client app
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
