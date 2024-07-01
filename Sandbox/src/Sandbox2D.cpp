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
	Z_PROFILE_FUNCTION();

	m_FPS = 1.0f / dt;

	{
		Z_PROFILE_SCOPE("Camera update");

		m_CameraController.OnUpdate(dt);
	}
	
	Zahra::Renderer2D::ResetStats();

	{
		Z_PROFILE_SCOPE("Renderer clear");

		Zahra::RenderCommand::SetClearColour(glm::make_vec4(m_ClearColour));
		Zahra::RenderCommand::Clear();
	}

	{
		Z_PROFILE_SCOPE("Renderer draw");

		Zahra::Renderer2D::BeginScene(m_CameraController.GetCamera());

		glm::vec2 dims = glm::make_vec2(m_QuadDimensions);
		glm::vec4 tint = glm::make_vec4(m_QuadColour);
		glm::vec2 pos  = glm::make_vec2(m_QuadPosition);

		int n = 20;

		Zahra::Renderer2D::DrawQuad({ 0.0f, 0.0f, -.1f }, 9.0f * dims, m_Texture);
		
		for (int i = 0; i < n*n; i++)
		{	
			glm::vec2 gridpoint(i % n, glm::floor((float)i / n));

			Zahra::Renderer2D::DrawRotatedQuad(
				pos + (10.0f / (float)n) * (gridpoint - glm::vec2(((float)n - 1) / 2,				// POSITION
				((float)n - 1) / 2)) * dims, dims * glm::vec2(10.0f / (float)n, 10.0f / (float)n),  // DIMENSIONS
				m_QuadRotation,																		// ROTATION
				tint * glm::vec4((1 / (float)n) * gridpoint, .4f, .8f)								// COLOUR
			);
		}
		
		Zahra::Renderer2D::EndScene();
	}

}

void Sandbox2DLayer::OnEvent(Zahra::Event& event)
{
	m_CameraController.OnEvent(event);
}

void Sandbox2DLayer::OnImGuiRender()
{
	Z_PROFILE_FUNCTION();

	ImGui::Begin("Scene Parameters");

	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

	ImGui::ColorEdit3("Background colour", m_ClearColour);

	ImGui::SliderFloat2("Quad position", m_QuadPosition, -5.0f, 5.0f);
	ImGui::SliderFloat2("Quad dimensions", m_QuadDimensions, .01f, 10.0f, "%.3f", 32);
	ImGui::SliderFloat("Quad rotation", &m_QuadRotation, -3.14f, 3.14f);
	ImGui::ColorEdit4("Quad tint", m_QuadColour);

	ImGui::Dummy({ 1,20 });
	
	ImGui::Text("FPS: %i", (int)m_FPS);
	ImGui::Text("Quads: %u", Zahra::Renderer2D::GetStats().QuadCount);
	ImGui::Text("Draw calls: %u", Zahra::Renderer2D::GetStats().DrawCalls);

	ImGui::End();

}

