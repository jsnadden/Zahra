#pragma once

#include <Zahra.h>

class Sandbox2DLayer : public Zahra::Layer
{
public:
	Sandbox2DLayer();

	~Sandbox2DLayer() = default;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float dt) override;
	void OnEvent(Zahra::Event& event) override;
	void OnImGuiRender() override;

	bool OnKeyPressedEvent(Zahra::KeyPressedEvent& event);

private:
	Zahra::OrthographicCameraController m_CameraController;

	Zahra::Ref<Zahra::Framebuffer> m_Framebuffer;

	float m_ClearColour[4] = { .114f, .820f, .69f, 1.0f };

	Zahra::Ref<Zahra::Texture2D> m_Texture;
	float m_QuadPosition[2] = { .0f, .0f };
	float m_QuadDimensions[2] = { 1.7f, 1.9f };
	float m_QuadRotation = .0f;
	float m_QuadColour[4] = { .878f, .718f, .172f, 1.0f };

	float m_FPS = .0f;


};

