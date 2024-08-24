#include "Sandbox2D.h"

#include <ImGui/imgui.h>
#include <glm/gtc/type_ptr.hpp>


Sandbox2DLayer::Sandbox2DLayer()
	: Layer("Sandbox2D_Layer")
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
	Zahra::RenderCommand::SetClearColour(glm::vec4(.0f));
	Zahra::RenderCommand::Clear();

}

void Sandbox2DLayer::OnEvent(Zahra::Event& event)
{
	Zahra::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<Zahra::KeyPressedEvent>(Z_BIND_EVENT_FN(Sandbox2DLayer::OnKeyPressedEvent));
}

void Sandbox2DLayer::OnImGuiRender()
{
	ImGui::Begin("Settings");

	ImGui::End();
}

bool Sandbox2DLayer::OnKeyPressedEvent(Zahra::KeyPressedEvent& event)
{
	return false;
}

