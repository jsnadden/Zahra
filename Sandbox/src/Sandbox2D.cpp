#include "Sandbox2D.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

Sandbox2DLayer::Sandbox2DLayer()
	: Layer("Sandbox2D_Layer"),
	m_CameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2DLayer::OnAttach()
{
	m_Texture = Zahra::Texture2D::Create("C:/dev/Zahra/Sandbox/assets/textures/yajirobe.png");
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
	Zahra::Renderer2D::DrawQuad(glm::make_vec2(m_SquarePosition), glm::make_vec2(m_SquareDimensions), glm::make_vec4(m_Colour2), m_SquareRotation);
	Zahra::Renderer2D::DrawQuad({.0f, .0f, .1f}, {526.0f / 841.0f, 1.0f}, m_Texture, glm::make_vec4(m_Colour3));
	Zahra::Renderer2D::EndScene();
}

void Sandbox2DLayer::OnEvent(Zahra::Event& event)
{
	m_CameraController.OnEvent(event);
}

void Sandbox2DLayer::OnImGuiRender()
{
	ImGui::Begin("Scene Parameters");

	ImGui::ColorEdit3("Background colour", m_Colour1);

	ImGui::ColorEdit4("Square colour", m_Colour2);
	ImGui::SliderFloat2("Square position", m_SquarePosition, -5.0f, 5.0f);
	ImGui::SliderFloat2("Square dimensions", m_SquareDimensions, .01f, 10.0f, "%.3f", 32);
	ImGui::SliderFloat("Square rotation", &m_SquareRotation, -3.14f, 3.14f);
	
	ImGui::ColorEdit4("Texture tint", m_Colour3);
	
	ImGui::End();
}