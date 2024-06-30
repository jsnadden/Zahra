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

	{
		Z_PROFILE_SCOPE("Camera update");

		m_CameraController.OnUpdate(dt);
	}
	
	{
		Z_PROFILE_SCOPE("Renderer clear");

		Zahra::RenderCommand::SetClearColour(glm::make_vec4(m_ClearColour));
		Zahra::RenderCommand::Clear();
	}

	{
		Z_PROFILE_SCOPE("Renderer draw");

		Zahra::Renderer2D::BeginScene(m_CameraController.GetCamera());

		glm::vec2 dims = glm::make_vec2(m_SquareDimensions);
		glm::vec4 tint = glm::make_vec4(m_SquareColour);

		int n = 10;
		for (float i = .0f; i < n*n; i++)
		{	
			glm::vec2 pos = glm::make_vec2(m_SquarePosition) + 2.0f * glm::vec2((int)i % n - n/2, (int)(i/n) - n/2);
			Zahra::Renderer2D::DrawQuad(.1f * pos, .1f * dims, m_Texture, glm::rotate(glm::mat4(1), i, glm::vec3(1,1,1)) * tint);
		}
			
			//Zahra::Renderer2D::DrawQuad({ .0f, .0f, .1f }, { 526.0f / 841.0f, 1.0f }, m_Texture, glm::make_vec4(m_Colour3), 1.0f);
		
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

	ImGui::ColorEdit3("Background colour", m_ClearColour);

	ImGui::ColorEdit4("Quad tint", m_SquareColour);
	ImGui::SliderFloat2("Quad position", m_SquarePosition, -5.0f, 5.0f);
	ImGui::SliderFloat2("Quad dimensions", m_SquareDimensions, .01f, 10.0f, "%.3f", 32);


	ImGui::End();

}

