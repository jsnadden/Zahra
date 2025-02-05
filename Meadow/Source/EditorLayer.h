#pragma once

#include "Editor/EditorEnums.h"
#include "UI/Elements/ColourDefs.h"
#include "UI/Panels/ContentBrowserPanel.h"
#include "UI/Panels/SceneHierarchyPanel.h"

#include <Zahra.h>

namespace Zahra
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer();

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(float dt) override;
		void OnImGuiRender() override;

		void OnEvent(Event& event) override;
		bool OnKeyPressedEvent(KeyPressedEvent& event);
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);
		bool OnWindowClosed(WindowClosedEvent& event);

	private:
		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene;

		// TODO: replace with a general SceneRenderer class including 2D and 3D rendering
		Ref<Renderer2D> m_Renderer2D;
		Ref<RenderPass> m_ClearPass;

		EditorCamera m_EditorCamera{ .5f, 1.78f, .1f, 1000.f };

		void ScenePlay();
		void SceneSimulate();
		void SceneStop();
		
		void UIMenuBar();
		void UIViewport();
		void UIStatsWindow(); // TODO: break up stats window into several tabs?
		
		void UIGizmo();
		TransformationType m_GizmoType = TransformationType::None;

		void UISceneControls();
		//std::map<std::string, Ref<Texture2D>> m_ControlIcons;
		//std::map<std::string, ImGuiTextureHandle> m_IconHandles;
		bool m_Paused = false;
		int32_t m_FramesPerStep = 10;
		int32_t m_StepCountdown = 0;

		// TODO: currently using bools to keep track of all these modals. figure out a smarter approach
		void UIAboutWindow();
		bool m_ShowAboutWindow = false;

		void UISaveChangesPrompt();
		void DoAfterHandlingUnsavedChanges(std::function<void()> callback, const std::string& message, bool canCancel);
		std::function<void()> m_AfterSaveChangesCallback;
		bool m_ShowSaveChangesPrompt = false;
		std::string m_SaveChangesPromptMessage;
		bool m_CanCancelSaveChangesPrompt = true;

		void UINewProjectWindow();
		bool m_ShowNewProjectWindow = false;

		void NewProject(const std::string& projectName, const std::filesystem::path& parentDirectory);
		void OpenProjectFile();
		bool OpenProjectFile(const std::filesystem::path& filepath);
		void SaveProjectFile();
		std::filesystem::path m_WorkingProjectFilepath;
		bool m_HaveActiveProject = false;
		bool TryLoadProjectScriptAssembly(const std::filesystem::path& filepath);

		void NewScene();
		void OpenSceneFile();
		bool OpenSceneFile(std::filesystem::path filepath);
		bool SaveSceneFile();
		bool SaveSceneFileAs();
		bool SaveSceneFileAs(const std::filesystem::path& filepath);
		// TODO: instead of the scene filepath, should save the scene's AssetID to config
		// (anyway for now a path, relative to the project directory)
		std::filesystem::path m_WorkingSceneRelativePath;

		bool m_AutosaveEnabled = false;
		Timer m_AutosaveTimer;

		void SaveEditorConfigFile();
		void LoadConfigFile();

		const float c_FramerateRefreshInterval = .5f;
		Timer m_FramerateRefreshTimer;
		float m_Framerate = .0f;

		// Viewport
		Ref<Image2D> m_ColourPickingAttachment;
		Ref<Framebuffer> m_ViewportFramebuffer;
		Ref<Texture2D> m_ViewportTexture;
		ImGuiTextureHandle m_ViewportTextureHandle = nullptr;
		Entity m_HoveredEntity;
		glm::vec2 m_ViewportSize = { 1280.0f, 720.0f };
		glm::vec2 m_ViewportBounds[2] = { {}, {} };
		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec4 m_HighlightSelectionColour = { MEADOW_YELLOW_1, 1.00f };

		void ReadHoveredEntity();

		// Editor panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		// TODO: make a general "Panel" class deriving from RefCounted, and replace these with Refs
	};
}

