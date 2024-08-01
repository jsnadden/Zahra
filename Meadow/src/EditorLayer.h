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
		EditorCamera m_EditorCamera;

		std::map<std::string, Ref<Texture2D>> m_Icons;

		enum class SceneState // TODO: move this to the scene class, add Paused?
		{
			Edit = 0, Play = 1
		};

		SceneState m_SceneState = SceneState::Edit;

		void OnScenePlay();
		void OnSceneStop();

		// Saving/opening scene files
		const wchar_t* m_FileTypesFilter[2] = { L"Zahra Scene", L"*.zsc" };
		std::filesystem::path m_CurrentFilePath;

		void UIMenuBar();
		void UIControls();
		void UIViewport();
		void UIGizmos();
		void UIStatsWindow(); // TODO: this should really be a bar, not a whole freaking window

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

		void ReceiveDragDrop();

		// Editor panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;

		

	};
}

