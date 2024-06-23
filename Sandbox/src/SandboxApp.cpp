#include <Zahra.h>

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ImGui/imgui.h"

class ExampleLayer : public Zahra::Layer
{
public:
	ExampleLayer()
		: Layer("Example_Layer"),
		m_Camera(-3.2f, 3.2f, -1.8f, 1.8f)
	{}

	void OnAttach() override
	{

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// DIAMONDS

		m_diamondVA = Zahra::VertexArray::Create();

		float diamondVertices[6 * 3] = {
			 0.50f,  0.00f, 0.0f,
			 0.00f,  0.87f, 0.0f,
			-0.50f,  0.00f, 0.0f,
			 0.50f,  0.00f, 0.0f,
			 0.00f, -0.87f, 0.0f,
			-0.50f,  0.00f, 0.0f
		};
		Zahra::Ref<Zahra::VertexBuffer> diamondVB = Zahra::VertexBuffer::Create(diamondVertices, sizeof(diamondVertices));

		{
			Zahra::BufferLayout layout = {
				{ Zahra::ShaderDataType::Float3, "a_Position" }
			};

			diamondVB->SetLayout(layout);
		}

		m_diamondVA->AddVertexBuffer(diamondVB);

		unsigned int diamondIndices[12] = { 0,1,2,3,4,5 };
		Zahra::Ref<Zahra::IndexBuffer> diamondIB = Zahra::IndexBuffer::Create(diamondIndices, sizeof(diamondIndices) / sizeof(uint32_t));
		m_diamondVA->SetIndexBuffer(diamondIB);


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// SQUARES

		m_squareVA = Zahra::VertexArray::Create();

		float squareVertices[4 * 5] = {
			 0.50f,  0.50f, 0.0f,  1.0f,  1.0f,
			-0.50f,  0.50f, 0.0f,  0.0f,  1.0f,
			-0.50f, -0.50f, 0.0f,  0.0f,  0.0f,
			 0.50f, -0.50f, 0.0f,  1.0f,  0.0f
		};
		Zahra::Ref<Zahra::VertexBuffer> squareVB = Zahra::VertexBuffer::Create(squareVertices, sizeof(squareVertices));

		{
			Zahra::BufferLayout layout = {
				{ Zahra::ShaderDataType::Float3, "a_Position" },
				{ Zahra::ShaderDataType::Float2, "a_TextureCoord" }
			};

			squareVB->SetLayout(layout);
		}

		m_squareVA->AddVertexBuffer(squareVB);

		unsigned int squareIndices[6] = { 0, 1, 2, 0, 2, 3 };
		Zahra::Ref<Zahra::IndexBuffer> squareIB = Zahra::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
		m_squareVA->SetIndexBuffer(squareIB);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// FLAT COLOUR SHADER

		std::string FlatColourVertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_PVMatrix;
			uniform mat4 u_Transform;

			void main()
			{
				gl_Position = u_PVMatrix * u_Transform * vec4(a_Position, 1.0);
			}
		)";
		std::string FlatColourFragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 colour;

			uniform vec4 u_Colour;

			void main()
			{
				colour = u_Colour;
			}
		)";

		m_FlatColourShader = Zahra::Shader::Create(FlatColourVertexSrc, FlatColourFragmentSrc);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TEXTURE SHADER

		/*std::string TextureVertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec2 a_TextureCoord;

			uniform mat4 u_PVMatrix;
			uniform mat4 u_Transform;
			
			out vec2 v_TextureCoord;

			void main()
			{
				v_TextureCoord = a_TextureCoord;
				gl_Position = u_PVMatrix * u_Transform * vec4(a_Position, 1.0);
			}
		)";
		std::string TextureFragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 colour;

			in vec2 v_TextureCoord;

			uniform sampler2D u_Texture;

			void main()
			{
				colour = texture(u_Texture, v_TextureCoord);
			}
		)";*/

		m_Texture = Zahra::Texture2D::Create("C:/dev/Zahra/Sandbox/assets/textures/cat.png");

		m_TextureShader = Zahra::Shader::Create("C:/dev/Zahra/Sandbox/assets/shaders/texture_shader.glsl");

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
		int dir = 1;

		if (Zahra::Input::IsKeyPressed(Z_KEY_W))  { dir =  1; camera_velocity += glm::rotate(glm::mat4(1.0f), m_Camera.GetRotation(), glm::vec3(.0f, .0f, 1.0f)) * glm::vec4(0, 1, 0, 1); }
		if (Zahra::Input::IsKeyPressed(Z_KEY_S))  { dir = -1; camera_velocity += glm::rotate(glm::mat4(1.0f), m_Camera.GetRotation(), glm::vec3(.0f, .0f, 1.0f)) * glm::vec4(0, -1, 0, 1); }
		if (Zahra::Input::IsKeyPressed(Z_KEY_D))  camera_velocity += glm::rotate(glm::mat4(1.0f), m_Camera.GetRotation(), glm::vec3(.0f, .0f, 1.0f)) * glm::vec4( 1,  0, 0, 1);
		if (Zahra::Input::IsKeyPressed(Z_KEY_A))  camera_velocity += glm::rotate(glm::mat4(1.0f), m_Camera.GetRotation(), glm::vec3(.0f, .0f, 1.0f)) * glm::vec4(-1,  0, 0, 1);

		if (Zahra::Input::IsKeyPressed(Z_KEY_SPACE)) m_Camera.SetPosition(glm::vec3(.0f));
		if (Zahra::Input::IsKeyPressed(Z_KEY_Q)) camera_angular_velocity += 1.0f;
		if (Zahra::Input::IsKeyPressed(Z_KEY_E)) camera_angular_velocity -= 1.0f;
		
		// Movement
		m_Camera.SetRotation(m_Camera.GetRotation() + dir * .5f * dt * camera_angular_velocity);
		m_Camera.SetPosition(m_Camera.GetPosition() + 1.0f * dt * camera_velocity);
		
		


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// RENDERING
		Zahra::RenderCommand::SetClearColour(glm::make_vec4(m_colour2));
		Zahra::RenderCommand::Clear();

		Zahra::Renderer::BeginScene(m_Camera);

		
		// diamonds
		{
			std::dynamic_pointer_cast<Zahra::OpenGLShader>(m_FlatColourShader)->Bind();
			std::dynamic_pointer_cast<Zahra::OpenGLShader>(m_FlatColourShader)->UploadUniformFloat4("u_Colour", glm::make_vec4(m_colour1));

			int N = 30;
			for (int i = 0; i < N; i++)
			{
				for (int j = 0; j < N; j++)
				{
					glm::vec3 position(i - .5f * j - .25f * N, .866f * j - .5f * N + .25f, 0);
					glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(.8f));
					Zahra::Renderer::Submit(m_FlatColourShader, m_diamondVA, transform);
				}
			}
		}

		
		// textured square
		{
			std::dynamic_pointer_cast<Zahra::OpenGLShader>(m_TextureShader)->Bind();

			m_Texture->Bind();
			std::dynamic_pointer_cast<Zahra::OpenGLShader>(m_TextureShader)->UploadUniformInt("u_Texture", 0);

			Zahra::Renderer::Submit(m_TextureShader, m_squareVA, glm::scale(glm::mat4(1.0f), glm::vec3(m_squareSize)));
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
		ImGui::SliderFloat("Square size", &m_squareSize, .1f, 10.0f, "%.3f", 32);
		ImGui::End();
	}

	private:
	Zahra::Ref<Zahra::Shader> m_FlatColourShader;
	Zahra::Ref<Zahra::Shader> m_TextureShader;
	Zahra::Ref<Zahra::VertexArray> m_diamondVA;
	Zahra::Ref<Zahra::VertexArray> m_squareVA;
	Zahra::OrthographicCamera m_Camera;
	Zahra::Ref<Zahra::Texture2D> m_Texture;

	glm::vec3 camera_velocity = glm::vec3(.0f);
	float camera_angular_velocity = .0f;

	float m_colour1[4] = { .114f, .820f, .69f, 1.0f };
	float m_colour2[4] = { .878f, .718f, .172f, 1.0f };

	float m_squareSize = 1.0f;

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