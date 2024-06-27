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


private:
	Zahra::OrthographicCameraController m_CameraController;

	float m_Colour1[4] = { .114f, .820f, .69f, 1.0f };
	float m_Colour2[4] = { .878f, .718f, .172f, 1.0f };
	float m_Colour3[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	Zahra::Ref<Zahra::Texture> m_Texture;

	float m_SquarePosition[2] = {.0f, .0f};
	float m_SquareDimensions[2] = {1.0f, 1.0f};
	float m_SquareRotation = .0f;

};

