#pragma once

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
		void OnEvent(Event& event) override;
		void OnImGuiRender() override;

		bool OnKeyPressedEvent(KeyPressedEvent& event);

	private:
		OrthographicCameraController m_CameraController;

		Ref<Framebuffer> m_Framebuffer;

		float m_ClearColour[4] = { .114f, .820f, .69f, 1.0f };

		Ref<Texture2D> m_Texture;
		float m_QuadPosition[2] = { .0f, .0f };
		float m_QuadDimensions[2] = { 1.7f, 1.9f };
		float m_QuadRotation = .0f;
		float m_QuadColour[4] = { .878f, .718f, .172f, 1.0f };

		float m_FPS = .0f;

		glm::vec2 m_ViewportSize = { 1280.0f, 720.0f };
		bool m_ViewportFocused = false, m_ViewportHovered = false;



	};
}

