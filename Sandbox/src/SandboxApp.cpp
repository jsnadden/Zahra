#include <Zahra.h>

#include "Platform/OpenGL/OpenGLShader.h"


#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui/imgui.h"

class ExampleLayer : public Zahra::Layer
{
public:
	ExampleLayer()
		: Layer("Example_Layer"),
		m_Camera(-3.2f, 3.2f, -1.8f, 1.8f)
	{}

	void OnAttach() override
	{

		m_VertexArray.reset(Zahra::VertexArray::Create());

		float vertices[6 * 7] = {
			 0.50f,  0.00f, 0.0f, .114f, .820f, .69f,  1.0f,
			 0.00f,  0.87f, 0.0f, .114f, .820f, .69f,  1.0f,
			-0.50f,  0.00f, 0.0f, .114f, .820f, .69f,  1.0f,
			 0.50f,  0.00f, 0.0f, .878f, .718f, .172f, 1.0f,
			 0.00f, -0.87f, 0.0f, .878f, .718f, .172f, 1.0f,
			-0.50f,  0.00f, 0.0f, .878f, .718f, .172f, 1.0f
		};
		std::shared_ptr<Zahra::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Zahra::VertexBuffer::Create(vertices, sizeof(vertices)));

		{
			Zahra::BufferLayout layout = {
				{ Zahra::ShaderDataType::Float3, "a_Position" },
				{ Zahra::ShaderDataType::Float4, "a_Colour" }
			};

			vertexBuffer->SetLayout(layout);
		}

		m_VertexArray->AddVertexBuffer(vertexBuffer);

		unsigned int indices[12] = { 0,1,2,3,4,5 };
		std::shared_ptr<Zahra::IndexBuffer> indexBuffer;
		indexBuffer.reset(Zahra::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		std::string vertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Colour;

			uniform mat4 u_PVMatrix;
			uniform mat4 u_Transform;

			out vec4 v_Colour;

			void main()
			{
				v_Colour = a_Colour;
				gl_Position = u_PVMatrix * u_Transform * vec4(a_Position, 1.0);
			}
		)";
		std::string fragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 colour;

			uniform vec4 u_Colour;
			
			in vec4 v_Colour;

			void main()
			{
				colour = u_Colour;
			}
		)";

		m_Shader.reset(Zahra::Shader::Create(vertexSrc, fragmentSrc));

	}

	void OnUpdate(float dt) override
	{
		//Z_TRACE("Framerate: {0}fps", 1/dt);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA

		// Inertia
		camera_velocity *= .89f;
		camera_angular_velocity *= .89f;

		// Acceleration
		int dir = 1.0f;

		if (Zahra::Input::GetMouseY() < 100 || Zahra::Input::IsKeyPressed(Z_KEY_W))  { dir =  1.0f; camera_velocity += glm::rotate(glm::mat4(1.0f), m_Camera.GetRotation(), glm::vec3(.0f, .0f, 1.0f)) * glm::vec4(0, 1, 0, 1); }
		if (Zahra::Input::GetMouseY() > 620 || Zahra::Input::IsKeyPressed(Z_KEY_S))  { dir = -1.0f; camera_velocity += glm::rotate(glm::mat4(1.0f), m_Camera.GetRotation(), glm::vec3(.0f, .0f, 1.0f)) * glm::vec4(0, -1, 0, 1); }
		if (Zahra::Input::GetMouseX() > 1080 || Zahra::Input::IsKeyPressed(Z_KEY_D))  camera_velocity += glm::rotate(glm::mat4(1.0f), m_Camera.GetRotation(), glm::vec3(.0f, .0f, 1.0f)) * glm::vec4( 1,  0, 0, 1);
		if (Zahra::Input::GetMouseX() < 200  || Zahra::Input::IsKeyPressed(Z_KEY_A))  camera_velocity += glm::rotate(glm::mat4(1.0f), m_Camera.GetRotation(), glm::vec3(.0f, .0f, 1.0f)) * glm::vec4(-1,  0, 0, 1);

		if (Zahra::Input::IsKeyPressed(Z_KEY_SPACE)) m_Camera.SetPosition(glm::vec3(.0f));
		if (Zahra::Input::IsKeyPressed(Z_KEY_Q)) camera_angular_velocity += 1.0f;
		if (Zahra::Input::IsKeyPressed(Z_KEY_E)) camera_angular_velocity -= 1.0f;
		
		// Movement
		m_Camera.SetRotation(m_Camera.GetRotation() + dir * .5f * dt * camera_angular_velocity);
		m_Camera.SetPosition(m_Camera.GetPosition() + 1.0f * dt * camera_velocity);
		//
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		

		Zahra::RenderCommand::SetClearColour(glm::make_vec4(m_colour2));
		Zahra::RenderCommand::Clear();

		Zahra::Renderer::BeginScene(m_Camera);

		std::dynamic_pointer_cast<Zahra::OpenGLShader>(m_Shader)->Bind();
		std::dynamic_pointer_cast<Zahra::OpenGLShader>(m_Shader)->UploadUniformFloat4("u_Colour", glm::make_vec4(m_colour1));

		int N = 30;
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				glm::vec3 position(i -.5f * j - .25f * N, .866f * j - .5f * N, 0);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(.8f));
				Zahra::Renderer::Submit(m_Shader, m_VertexArray, transform);
			}
		}

		

		Zahra::Renderer::EndScene();
	}

	void OnEvent(Zahra::Event& event) override
	{
		Zahra::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Zahra::KeyPressedEvent>(Z_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));
	}

	bool OnKeyPressedEvent(Zahra::KeyPressedEvent& event)
	{
		// handle DISCRETE key presses/repeats

		return false;
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Colour Scheme");
		ImGui::ColorEdit3("Tiles", m_colour1);
		ImGui::ColorEdit3("Background", m_colour2);
		ImGui::End();
	}

	private:
	std::shared_ptr<Zahra::Shader> m_Shader;
	std::shared_ptr<Zahra::VertexArray> m_VertexArray;
	Zahra::OrthographicCamera m_Camera;

	glm::vec3 camera_velocity = glm::vec3(.0f);
	float camera_angular_velocity = .0f;

	float m_colour1[4] = { .114f, .820f, .69f, 1.0f };
	float m_colour2[4] = { .878f, .718f, .172f, 1.0f };

};





class Sandbox : public Zahra::Application
{
public:

	Sandbox()
		: Zahra::Application()
	{
		PushLayer(new ExampleLayer());

		
	}

	~Sandbox()
	{

	}


};

Zahra::Application* Zahra::CreateApplication()
{
	return new Sandbox();
}