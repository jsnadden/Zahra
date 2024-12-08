#include "Sandbox.h"

#include "Zahra/Renderer/Shader.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

SandboxLayer::SandboxLayer()
	: Layer("Sandbox_Layer")
{
}

void SandboxLayer::OnAttach()
{
	
}

void SandboxLayer::OnDetach()
{
	
}

void SandboxLayer::OnUpdate(float dt)
{
	Zahra::Renderer::DrawTutorialScene(m_ViewportTexture2D);

	/*if (!m_ViewportImGuiTextureHandle)
		m_ViewportImGuiTextureHandle = Zahra::Application::Get().GetImGuiLayer()->RegisterTexture(m_ViewportTexture2D);*/
}

void SandboxLayer::OnEvent(Zahra::Event& event)
{
	Zahra::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<Zahra::KeyPressedEvent>(Z_BIND_EVENT_FN(SandboxLayer::OnKeyPressedEvent));
}

void SandboxLayer::OnImGuiRender()
{
	if (ImGui::Begin("Controls"))
	{
		//ImGui::Image(m_ViewportImGuiTextureHandle, ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
	}
	
}

bool SandboxLayer::OnKeyPressedEvent(Zahra::KeyPressedEvent& event)
{
	return false;
}

