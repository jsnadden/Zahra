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
		m_CameraController(1280.0f / 720.0f, true)
	{}

	void OnAttach() override
	{

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// DIAMOND GEOMETRY

		m_DiamondVertexArray = Zahra::VertexArray::Create();

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

		m_DiamondVertexArray->AddVertexBuffer(diamondVB);

		unsigned int diamondIndices[12] = { 0,1,2,3,4,5 };
		Zahra::Ref<Zahra::IndexBuffer> diamondIB = Zahra::IndexBuffer::Create(diamondIndices, sizeof(diamondIndices) / sizeof(uint32_t));
		m_DiamondVertexArray->SetIndexBuffer(diamondIB);


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// SQUARE GEOMETRY

		m_SquareVertexArray = Zahra::VertexArray::Create();

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

		m_SquareVertexArray->AddVertexBuffer(squareVB);

		unsigned int squareIndices[6] = { 0, 1, 2, 0, 2, 3 };
		Zahra::Ref<Zahra::IndexBuffer> squareIB = Zahra::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
		m_SquareVertexArray->SetIndexBuffer(squareIB);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// SHADERS

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

		m_FlatColourShader = Zahra::Shader::Create("FlatColourShader", FlatColourVertexSrc, FlatColourFragmentSrc);

		m_Texture = Zahra::Texture2D::Create("C:/dev/Zahra/Sandbox/assets/textures/yajirobe.png");
		m_Texture->Bind();

		auto textureShader = m_ShaderLibrary.Load("TextureShader", "C:/dev/Zahra/Sandbox/assets/shaders/texture_shader.glsl");
		std::dynamic_pointer_cast<Zahra::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<Zahra::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);

	}

	void OnUpdate(float dt) override
	{
		Zahra::RenderCommand::SetClearColour(glm::make_vec4(m_Colour2));
		Zahra::RenderCommand::Clear();

		Zahra::Renderer3D::BeginScene(m_CameraController.GetCamera());

		m_CameraController.OnUpdate(dt);
		
		//////////////////////////////////////////////////////////////////////////////////////////////////
		// DIAMONDS
		std::dynamic_pointer_cast<Zahra::OpenGLShader>(m_FlatColourShader)->Bind();
		std::dynamic_pointer_cast<Zahra::OpenGLShader>(m_FlatColourShader)->UploadUniformFloat4("u_Colour", glm::make_vec4(m_Colour1));

		int N = 30;
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				glm::vec3 position(i - .5f * j - .25f * N, .866f * j - .5f * N + .25f, 0);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(.8f));
				Zahra::Renderer3D::Submit(m_FlatColourShader, m_DiamondVertexArray, transform);
			}
		}
		//////////////////////////////////////////////////////////////////////////////////////////////////
		// TEXTURE
		Zahra::Renderer3D::Submit(m_ShaderLibrary.Get("TextureShader"), m_SquareVertexArray, glm::scale(glm::mat4(1.0f), glm::vec3(m_SquareSize)));
		//////////////////////////////////////////////////////////////////////////////////////////////////

		Zahra::Renderer3D::EndScene();

	}

	void OnEvent(Zahra::Event& event) override
	{
		Zahra::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Zahra::KeyPressedEvent>(Z_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));

		m_CameraController.OnEvent(event);
	}

	bool OnKeyPressedEvent(Zahra::KeyPressedEvent& event)
	{
		// handle DISCRETE key presses/repeats

		return false;
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Colour Scheme");
		ImGui::ColorEdit3("Tiles", m_Colour1);
		ImGui::ColorEdit3("Background", m_Colour2);
		ImGui::SliderFloat("Square size", &m_SquareSize, .1f, 10.0f, "%.3f", 32);
		ImGui::End();
	}

private:
	Zahra::ShaderLibrary m_ShaderLibrary;

	Zahra::Ref<Zahra::Shader> m_FlatColourShader;
	
	Zahra::Ref<Zahra::VertexArray> m_DiamondVertexArray;
	Zahra::Ref<Zahra::VertexArray> m_SquareVertexArray;
	
	Zahra::Ref<Zahra::Texture2D> m_Texture;

	Zahra::OrthographicCameraController m_CameraController;

	float m_Colour1[4] = { .114f, .820f, .69f, 1.0f };
	float m_Colour2[4] = { .878f, .718f, .172f, 1.0f };
	float m_SquareSize = 1.0f;

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