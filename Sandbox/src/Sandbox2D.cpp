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
	Zahra::ShaderSpecification shaderSpec{};
	shaderSpec.Name = "vulkan_tutorial";
	shaderSpec.SourceDirectory = "Resources/Shaders";
	m_Shader = Zahra::Shader::Create(shaderSpec);

	Zahra::PipelineSpecification pipelineSpec{};
	pipelineSpec.Shader = m_Shader;
	m_Pipeline = Zahra::Pipeline::Create(pipelineSpec);
}

void Sandbox2DLayer::OnDetach()
{
	m_Shader.Reset();
	m_Pipeline.Reset();
}

void Sandbox2DLayer::OnUpdate(float dt)
{
	// 1) get swapchain image
	// 2) record commandbuffer commands using that image
	// 3) send the image back to the swapchain for presentation

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

