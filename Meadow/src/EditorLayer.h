#pragma once

#include <Zahra.h>

#include "Panels/SceneHierarchyPanel.h"

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

	private:

		Ref<Scene> m_ActiveScene;

		// Viewport
		Ref<Framebuffer> m_Framebuffer;
		glm::vec2 m_ViewportSize = { 1280.0f, 720.0f };
		bool m_ViewportFocused = false, m_ViewportHovered = false;
		float m_ClearColour[4] = { .0f, .0f, .0f, 1.0f };//{ .114f, .820f, .69f, 1.0f };
		
		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;



		// TEMP
		Entity m_Quad;
		Entity m_Camera;

		



	};
}

