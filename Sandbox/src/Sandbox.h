#pragma once

#include <Zahra.h>

class SandboxLayer : public Zahra::Layer
{
public:
	SandboxLayer();

	~SandboxLayer() = default;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float dt) override;
	void OnEvent(Zahra::Event& event) override;
	void OnImGuiRender() override;

	bool OnKeyPressedEvent(Zahra::KeyPressedEvent& event);
	bool OnWindowResizedEvent(Zahra::WindowResizedEvent& event);

private:
	Zahra::Ref<Zahra::Renderer2D> m_Renderer2D;

	bool m_Toggle = true;

};

