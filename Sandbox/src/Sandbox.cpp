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
	uint32_t framesInFlight = Zahra::Renderer::GetFramesInFlight();
	m_ViewportTexture2D.resize(framesInFlight);
	m_ViewportImGuiTextureHandle.resize(framesInFlight);
}

void SandboxLayer::OnDetach()
{
	
}

void SandboxLayer::OnUpdate(float dt)
{
	m_FrameIndex = Zahra::Renderer::GetCurrentFrameIndex();

	Zahra::Renderer::DrawTutorialScene();

	/*if (!m_ViewportImGuiTextureHandle[m_FrameIndex])
		m_ViewportImGuiTextureHandle[m_FrameIndex] = Zahra::Application::Get().GetImGuiLayer()->RegisterTexture(m_ViewportTexture2D[m_FrameIndex]);*/
}

void SandboxLayer::OnEvent(Zahra::Event& event)
{
	Zahra::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<Zahra::KeyPressedEvent>(Z_BIND_EVENT_FN(SandboxLayer::OnKeyPressedEvent));
}

void SandboxLayer::OnImGuiRender()
{
	if (ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoCollapse))
	{
		//ImGui::Image(m_ViewportImGuiTextureHandle[m_FrameIndex], ImGui::GetContentRegionAvail(), ImVec2(0, 0), ImVec2(1, 1));
		ImGui::End();
	}
	
}

bool SandboxLayer::OnKeyPressedEvent(Zahra::KeyPressedEvent& event)
{
	return false;
}

