#pragma once

#include "Zahra/Core/Defines.h"
#include "Zahra/Core/LayerStack.h"
#include "Zahra/Core/Window.h"
#include "Zahra/Events/ApplicationEvent.h"
#include "Zahra/Events/Event.h"
#include "Zahra/ImGui/ImGuiLayer.h"
#include "Zahra/Renderer/RendererConfig.h"

namespace Zahra
{
	/**
	 * @brief Wraps an array of command-line arguments (encoded as char arrays).
	 */
	struct ApplicationCommandLineArgs
	{
		int Count = 0; /**< @brief The number of arguments */
		char** Args = nullptr; /**< @brief Array of arguments */

		/**
		 * @brief Indexes into Args array.
		 */
		const char* operator[](int index) const
		{
			Z_CORE_ASSERT(index < Count);
			return Args[index];
		}
	};

	/**
	 * @brief Contains an application's version number(s).
	 */
	struct ApplicationVersion
	{
		uint16_t Major = 0;
		uint16_t Minor = 0;
		uint16_t Patch = 0;

		ApplicationVersion() = default;
		ApplicationVersion(int major, int minor, int patch)
			: Major(major), Minor(minor), Patch(patch) {}
	};

	/**
	 * @brief Contains a collection of requirements an application might impose on graphics hardware.
	 * 
	 * To be used by the Renderer system when selecting a physical GPU device, to determine
	 * suitability for app-specific needs.
	 * Default values for member variables are intended as a baseline of minimal requirements
	 * for the engine itself.
	 */
	struct GPURequirements
	{
		// choose default values to trivialise checks, and make sure to add the
		// corresponding tests in VulkanSwapchain::MeetsMinimimumRequirements
		
		bool IsDiscreteGPU = false; /**< @brief Does the app require a discreet (as opposed to on-board) GPU? */
		bool AnisotropicFiltering = false; /**< @brief Should the graphics hardware be capable of anisotropic texture filtering? */

		uint32_t MinBoundTextureSlots = 1; /**< @brief How many texture binding slots does the app need access to? */
	};

	/**
	 * @brief Contains the data needed to specify construction of an instance of Application.
	 */
	struct ApplicationSpecification
	{
		std::string Name = "Zahra_App"; /**< @brief The app's name */
		ApplicationVersion Version; /**< @brief The app's current version number(s) */

		std::filesystem::path WorkingDirectory; /**< @brief Filepath of the application's working directory, relative to the engine's root directory */
		ApplicationCommandLineArgs CommandLineArgs;  /**< @brief Any command line arguments supplied at app launch */

		bool IsEditor = false;  /**< @brief Is this a level editor? This enables certain features that should be stripped from a runtime app. */

		// TODO: find these a home
		std::filesystem::path ShaderSourceDirectory; /**< @brief Temporary, belongs elsewhere */
		std::filesystem::path ShaderCacheDirectory; /**< @brief Temporary, belongs elsewhere */

		RendererConfig RendererConfig; /**< @brief App-specific configuration data for the Renderer system*/
		GPURequirements GPURequirements; /**< @brief The app's minimal GPU requirement data */

		ImGuiLayerConfig ImGuiConfig;  /**< @brief App-specific configuration data for the engine's ImGui overlay */
	};

	/**
	 * @brief Represents a front-end application for the engine, controlling runtime behaviour.
	 * 
	 * The app itself should define its own class inheriting from this, though still call the below constructor.
	 */
	class Application
	{
	public:
		/**
		 * @brief The engine's ignition key!
		 *
		 * Initialises all engine subsystems.
		 * Inheriting classes should call this in their initialiser list.
		 *
		 * @param spec App-specific engine data, supplied by a child class.
		 */
		Application(const ApplicationSpecification& spec);
		virtual ~Application();

		void Run(); /**< @brief This is called by the main function, beginning the application's runtime loop */

		void SubmitToMainThread(const std::function<void()>& command); /**< @brief Called to submit a command to be run on the application's main thread, at the top of the next frame. */

		/**
		 * @brief A callback function to handle event data coming from the OS, e.g. user input.
		 * 
		 * At the top of each frame this function will be called for each queued
		 * event (this should be handled by the chosen windowing library).
		 * 
		 * @param e The polled Event.
		 */
		void OnEvent(Event& e);

		void PushLayer(Layer* layer); /**< @brief Adds a new Layer to the app's LayerStack */
		void PushOverlay(Layer* overlay); /**< @brief Adds a new overlay Layer to the app's LayerStack */

		static inline Application& Get() { return *s_Instance; }  /**< @brief Retrieve the static Application (child) instance */

		const ApplicationSpecification& GetSpecification() const { return m_Specification; } /**< @brief Retrieve this instance's spec data */
		inline Window& GetWindow() { return *m_Window; } /**< @brief Retrieve the current Window instance */
		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; } /**< @brief Retrieve the ImGuiLayer representing the engine's primary UI overlay */

		void Exit(); /**< @brief Request the program terminate at the end of the current frame */

	private:
		ApplicationSpecification m_Specification;
		static Application* s_Instance;

		std::vector<std::function<void()>> m_MainThreadCommandQueue;
		std::mutex m_MainThreadCommandQueueMutex;

		Scope<Window> m_Window;

		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer = nullptr;

		bool m_Running = true;
		bool m_Minimised = false;

		float m_PreviousFrameStartTime = .0f;

		void FlushCommandQueue();

		bool OnWindowClosed(WindowClosedEvent& e);
		bool OnWindowResized(WindowResizedEvent& e);
		bool OnWindowMinimised(WindowMinimisedEvent& e);
	};

	/**
	 * @brief Provides app-specific initialisation instructions.
	 * 
	 * Not defined in the core engine, but rather left for the front-end app to specify.
	 * Primarily this should fill out an ApplicationSpecification struct, use that to
	 * construct an Application (child) instance, and return its address.
	 * 
	 * @param args The command-line arguments received from the main function.
	 * @return Pointer to the constructed Application (child) instance.
	 */
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
