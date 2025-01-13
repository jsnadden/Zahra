#pragma once

#include "Utils/TypeDefs.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/SceneHierarchyPanel.h"

#include <Zahra.h>

namespace Zahra
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer() = default;

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

		const wchar_t* m_FileTypesFilter[2] = { L"Zahra Scene", L"*.zsc" };
		std::filesystem::path m_WorkingSceneFilepath;

		// TODO: replace with a general SceneRenderer class including 2D and 3D rendering
		Ref<Renderer2D> m_Renderer2D;
		Ref<RenderPass> m_ClearPass;

		EditorCamera m_EditorCamera{ .5f, 1.78f, .1f, 1000.f };

		void ScenePlay();
		void SceneSimulate();
		void SceneStop();
		
		void UIMenuBar();
		void UIAboutWindow(); bool m_ShowAboutWindow = false;
		void UIControls();
		void UIViewport();
		void UIGizmo();
		void UIStatsWindow(); // TODO: break up stats window into several tabs?
		void UISaveChangesPrompt();

		std::map<std::string, Ref<Texture2D>> m_Icons;
		std::map<std::string, ImGuiTextureHandle> m_IconHandles;

		void DoAfterHandlingUnsavedChanges(std::function<void()> callback);
		bool m_ShowSaveChangesPrompt = false;
		std::function<void()> m_SaveChangesCallback;

		void NewScene();
		void OpenSceneFile();
		void OpenSceneFile(std::filesystem::path filepath);
		bool SaveSceneFile();
		bool SaveAsSceneFile();

		void WriteConfigFile();
		void ReadConfigFile();

		Timer m_SceneCacheTimer;
		uint32_t m_SceneCacheIndex = 0;
		void CacheWorkingScene();

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
		glm::vec4 m_HighlightSelectionColour = { 0.92f, 0.72f, 0.18f, 1.00f };

		// Transform gizmos
		TransformationType m_GizmoType = TransformationType::None;

		void ReadHoveredEntity();

		// Editor panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;

	};
}

