#include "Sandbox2D.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

Sandbox2DLayer::Sandbox2DLayer()
	: Layer("Sandbox2D_Layer"),
	m_CameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2DLayer::OnAttach()
{
	
}

void Sandbox2DLayer::OnDetach()
{

}

void Sandbox2DLayer::OnUpdate(float dt)
{
	m_CameraController.OnUpdate(dt);

	Zahra::RenderCommand::SetClearColour(glm::make_vec4(m_Colour1));
	Zahra::RenderCommand::Clear();

	Zahra::Renderer2D::BeginScene(m_CameraController.GetCamera());
	Zahra::Renderer2D::DrawQuad({ .0f, .0f, .0f }, { 1.0f, 1.0f }, glm::make_vec4(m_Colour2));
	Zahra::Renderer2D::EndScene();
}

void Sandbox2DLayer::OnEvent(Zahra::Event& event)
{
	m_CameraController.OnEvent(event);
}

void Sandbox2DLayer::OnImGuiRender()
{
	ImGui::Begin("Colour Scheme");
	ImGui::ColorEdit3("Background", m_Colour1);
	ImGui::ColorEdit4("Square", m_Colour2);
	ImGui::End();
}