#include "Sandbox.h"

#include "Zahra/Renderer/Shader.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "Windows.h"

SandboxLayer::SandboxLayer()
	: Layer("Sandbox_Layer")
{
}

void SandboxLayer::OnAttach()
{
	/*uint32_t framesInFlight = Zahra::Renderer::GetFramesInFlight();
	m_ViewportTexture2D.resize(framesInFlight);
	m_ViewportImGuiTextureHandle.resize(framesInFlight);*/
}

void SandboxLayer::OnDetach()
{
	
}

void SandboxLayer::OnUpdate(float dt)
{
	/*Zahra::Renderer::DrawTutorialScene();

	int n = 20;
	float scale = 10.0f / n;

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			float x = -5.0f + (i + 0.5f) * scale;
			float y = -5.0f + (j + 0.5f) * scale;
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), { x, y, 0 }) * glm::scale(glm::mat4(1.0f), {scale, scale, scale});
			glm::vec4 colour = { .25f + .5f * ((float)i) / n, .25f + .5f * ((float)j) / n, .1f, 1.0f };
			Zahra::Renderer::DrawQuad(transform, colour);
		}
	}*/
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

