#pragma once

#include <Zahra.h>

#include "Panels/SceneHierarchyPanel.h"

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

		// Saving/opening scene files
		const char* m_FileTypesFilter = "Zahra Scene (*.zsc)\0*.zsc\0\0";
		std::optional<std::string> m_CurrentFilePath = std::nullopt;

		void NewScene();
		void OpenSceneFile();
		void SaveSceneFile();
		void SaveAsSceneFile();

		// Viewport
		Ref<Framebuffer> m_Framebuffer;
		glm::vec2 m_ViewportSize = { 1280.0f, 720.0f };
		glm::vec2 m_ViewportBounds[2];
		bool m_ViewportFocused = false, m_ViewportHovered = false;
		float m_ClearColour[4] = { .0f, .0f, .0f, 1.0f };
		int m_GizmoType = -1;
		Entity m_HoveredEntity;

		void ReadHoveredEntity();
		void RenderGizmos();

		// Editor panels
		SceneHierarchyPanel m_SceneHierarchyPanel;


	};
}

