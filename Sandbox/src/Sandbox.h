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
	void OnImGuiRender() override;

	void OnEvent(Zahra::Event& event) override;
	bool OnKeyPressedEvent(Zahra::KeyPressedEvent& event);
	bool OnWindowResizedEvent(Zahra::WindowResizedEvent& event);

private:
	Zahra::Ref<Zahra::Renderer2D> m_Renderer2D;

	Zahra::EditorCamera m_Camera{ .5f, 1.78f, .1f, 100.f };

	uint32_t m_ViewportWidth = 1, m_ViewportHeight = 1;
	/*Zahra::Ref<Zahra::Image2D> m_ViewportRenderTarget;
	Zahra::Ref<Zahra::Framebuffer> m_ClearViewportFramebuffer, m_ViewportFramebuffer;
	std::vector<Zahra::Ref<Zahra::Texture2D>> m_Textures;*/

	Zahra::Ref<Zahra::Scene> m_Scene;
	std::vector<std::vector<Zahra::Entity>> m_EntityGrid;

	const float c_FramerateRefreshInterval = .5f;
	Zahra::Timer m_FramerateRefreshTimer;
	float m_Framerate = .0f;

	bool m_Toggle = true;

};

