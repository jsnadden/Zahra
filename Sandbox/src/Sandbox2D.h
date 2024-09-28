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
	Zahra::Ref<Zahra::Shader> m_Shader;
	Zahra::Ref<Zahra::RenderPass> m_RenderPass;
	Zahra::Ref<Zahra::Pipeline> m_Pipeline;
};

