#pragma once

#include <Zahra.h>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"

#include <optional>

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
		void OnEvent(Event& event) override;
		void OnImGuiRender() override;
		bool OnKeyPressedEvent(KeyPressedEvent& event);
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);

	private:

		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene;

		EditorCamera m_EditorCamera{ .5f, 1.78f, .1f, 1000.f };

		std::map<std::string, Ref<Texture2D>> m_Icons;

		enum class SceneState // TODO: add Paused?
		{
			Edit = 0, Play = 1, Simulate = 2
		};

		SceneState m_SceneState = SceneState::Edit;

		void ScenePlay();
		void SceneSimulate();
		void SceneStop();

		// Saving/opening scene files
		const wchar_t* m_FileTypesFilter[2] = { L"Zahra Scene", L"*.zsc" };
		std::filesystem::path m_CurrentFilePath;

		void UIMenuBar();
		void UIAboutWindow(); bool m_ShowAboutWindow = false;
		void UIControls();
		void UIViewport();
		void UIGizmos();
		void UIHighlightSelection();
		void UIStatsWindow(); // TODO: do something better with this stuff

		glm::vec4 m_HighlightSelectionColour = { 0.92f, 0.72f, 0.18f, 1.00f };

		void NewScene();
		void OpenSceneFile();
		void OpenSceneFile(std::filesystem::path filepath);
		void SaveSceneFile();
		void SaveAsSceneFile();

		// Viewport
		Ref<Framebuffer> m_Framebuffer;
		glm::vec2 m_ViewportSize = { 1280.0f, 720.0f };
		glm::vec2 m_ViewportBounds[2] = { {}, {} };
		bool m_ViewportFocused = false, m_ViewportHovered = false;
		float m_ClearColour[4] = { .0f, .0f, .0f, 1.0f };
		int m_GizmoType = -1;
		Entity m_HoveredEntity;

		void ReadHoveredEntity();

		// Editor panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;

		

	};
}

