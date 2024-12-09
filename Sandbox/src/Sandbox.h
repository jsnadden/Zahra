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
	uint32_t m_FrameIndex;

	std::vector<Zahra::Ref<Zahra::Texture2D>> m_ViewportTexture2D;
	std::vector<void*> m_ViewportImGuiTextureHandle;

};

