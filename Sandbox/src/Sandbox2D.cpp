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
	m_SquareVertexArray = Zahra::VertexArray::Create();

	float squareVertices[4 * 3] = {
		 0.50f,  0.50f, 0.0f,
		-0.50f,  0.50f, 0.0f,
		-0.50f, -0.50f, 0.0f,
		 0.50f, -0.50f, 0.0f
	};
	Zahra::Ref<Zahra::VertexBuffer> squareVB = Zahra::VertexBuffer::Create(squareVertices, sizeof(squareVertices));

	{
		Zahra::BufferLayout layout = {
			{ Zahra::ShaderDataType::Float3, "a_Position" }
		};

		squareVB->SetLayout(layout);
	}

	m_SquareVertexArray->AddVertexBuffer(squareVB);

	unsigned int squareIndices[6] = { 0, 1, 2, 0, 2, 3 };
	Zahra::Ref<Zahra::IndexBuffer> squareIB = Zahra::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
	m_SquareVertexArray->SetIndexBuffer(squareIB);

	auto shader = m_ShaderLibrary.Load("FlatColourShader", "C:/dev/Zahra/Sandbox/assets/shaders/flatcolour.glsl");
	std::dynamic_pointer_cast<Zahra::OpenGLShader>(shader)->Bind();

}

void Sandbox2DLayer::OnDetach()
{

}


void Sandbox2DLayer::OnUpdate(float dt)
{
	Zahra::RenderCommand::SetClearColour(glm::make_vec4(m_Colour1));
	Zahra::RenderCommand::Clear();

	Zahra::Renderer::BeginScene(m_CameraController.GetCamera());

	std::dynamic_pointer_cast<Zahra::OpenGLShader>(m_ShaderLibrary.Get("FlatColourShader"))
		->UploadUniformFloat4("u_Colour", glm::make_vec4(m_Colour2));
	Zahra::Renderer::Submit(m_ShaderLibrary.Get("FlatColourShader"), m_SquareVertexArray, glm::mat4(1.0f));


	m_CameraController.OnUpdate(dt);
	Zahra::Renderer::EndScene();
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