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
		float m_ClearColour[4] = { .114f, .820f, .69f, 1.0f };
		
		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;



		// TEMP
		Entity m_QuadEntity;
		bool m_CameraToggle = false;
		Entity m_FixedCamera;
		Entity m_DynamicCamera;
		Ref<Texture2D> m_Texture;
		float m_QuadPosition[3] = { .0f, .0f, -.5f };
		float m_QuadDimensions[3] = { 1.0f, 1.0f, 1.0f };
		float m_QuadRotation = .0f;
		float m_QuadColour[4] = { .878f, .718f, .172f, 1.0f };
		float m_FPS = .0f;

		



	};
}

