#include "Sandbox2D.h"

#include "Zahra/Renderer/Shader.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

Sandbox2DLayer::Sandbox2DLayer()
	: Layer("Sandbox2D_Layer")
{
}

void Sandbox2DLayer::OnAttach()
{
	Zahra::Shader::ShaderSpecification spec;
	spec.Name = "vulkan_tutorial";
	spec.SourceDirectory = "Assets/Shaders";

	m_Shader = Zahra::Shader::Create(spec);
}

void Sandbox2DLayer::OnDetach()
{
	m_Shader.Reset();
}

void Sandbox2DLayer::OnUpdate(float dt)
{
	Zahra::RenderCommand::SetClearColour(glm::vec4(.1f));
	Zahra::RenderCommand::Clear();

}

void Sandbox2DLayer::OnEvent(Zahra::Event& event)
{
	Zahra::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<Zahra::KeyPressedEvent>(Z_BIND_EVENT_FN(Sandbox2DLayer::OnKeyPressedEvent));
}

void Sandbox2DLayer::OnImGuiRender()
{
	// TODO: ressurect
	/*if (ImGui::Begin("Controls"))
	{
		ImGui::End();
	}*/
	
}

bool Sandbox2DLayer::OnKeyPressedEvent(Zahra::KeyPressedEvent& event)
{
	return false;
}

