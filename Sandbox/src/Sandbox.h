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

private:
	Zahra::Ref<Zahra::Texture2D> m_ViewportTexture2D;
	void* m_ViewportImGuiTextureHandle = nullptr;

};

