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

	void OnViewportResize();

	void OnEvent(Zahra::Event& event) override;
	bool OnKeyPressedEvent(Zahra::KeyPressedEvent& event);
	bool OnWindowResizedEvent(Zahra::WindowResizedEvent& event);

private:

	Zahra::EditorCamera m_Camera{ .5f, 1.78f, .1f, 100.f };

	uint32_t m_ViewportWidth = 1, m_ViewportHeight = 1;
	Zahra::Ref<Zahra::Image2D> m_ViewportRenderTarget;
	Zahra::Ref<Zahra::Texture2D> m_ViewportTexture;
	Zahra::Ref<Zahra::Framebuffer> m_ViewportFramebuffer;

	// TEMPORARY
	Zahra::Ref<Zahra::RenderPass> m_ClearPass;

	Zahra::Ref<Zahra::Scene> m_Scene;
	Zahra::Ref<Zahra::Renderer2D> m_Renderer2D;

	std::vector<std::vector<Zahra::Entity>> m_EntityGrid;

	const float c_FramerateRefreshInterval = .5f;
	Zahra::Timer m_FramerateRefreshTimer;
	float m_Framerate = .0f;

};

