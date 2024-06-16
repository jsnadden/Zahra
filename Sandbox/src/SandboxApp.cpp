#include <Zahra.h>
#include <glm/gtc/matrix_transform.hpp>

class ExampleLayer : public Zahra::Layer
{
public:
	ExampleLayer() : Layer("Example_Layer"), m_Camera(-3.2f, 3.2f, -1.8f, 1.8f) {}

	void OnAttach() override
	{

		m_VertexArray.reset(Zahra::VertexArray::Create());

		float vertices[6 * 7] = {
			 1.00f,  0.00f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.50f,  0.87f, 0.0f, 0.7f, 0.7f, 0.0f, 1.0f,
			-0.50f,  0.87f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			-1.00f,  0.00f, 0.0f, 0.0f, 0.7f, 0.7f, 1.0f,
			-0.50f, -0.87f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			 0.50f, -0.87f, 0.0f, 0.7f, 0.0f, 0.7f, 1.0f
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

		unsigned int indices[12] = { 0,1,2,0,2,3,0,3,4,0,4,5 };
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
			
			in vec4 v_Colour;

			void main()
			{
				colour = v_Colour;
			}
		)";

		m_Shader.reset(new Zahra::Shader(vertexSrc, fragmentSrc));

	}

	void OnUpdate(float dt) override
	{
		//Z_TRACE("Framerate: {0}fps", 1/dt);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA

		// Inertia
		camera_velocity = 0.8f * camera_velocity;

		// Acceleration
		if (Zahra::Input::GetMouseY() < 100)  camera_velocity += glm::vec4( 0,  1, 0, 1);
		if (Zahra::Input::GetMouseY() > 620)  camera_velocity += glm::vec4( 0, -1, 0, 1);
		if (Zahra::Input::GetMouseX() > 1080) camera_velocity += glm::vec4( 1,  0, 0, 1);
		if (Zahra::Input::GetMouseX() < 200)  camera_velocity += glm::vec4(-1,  0, 0, 1);

		if (Zahra::Input::IsKeyPressed(Z_KEY_SPACE)) m_Camera.SetPosition(hex_position);
		
		// Movement
		m_Camera.SetPosition(m_Camera.GetPosition() + .8f * dt * camera_velocity);

		//
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// HEXAGON

		// Reset velocity
		hex_speed = .0f;
		hex_angular_velocity = .0f;

		// Acceleration
		forward = 1;
		if (Zahra::Input::IsKeyPressed(Z_KEY_W)) hex_speed =  1.5f;
		if (Zahra::Input::IsKeyPressed(Z_KEY_S)) { hex_speed = -1.5f; forward = -1; }
		if (Zahra::Input::IsKeyPressed(Z_KEY_A)) hex_angular_velocity =  forward * 2.5f;
		if (Zahra::Input::IsKeyPressed(Z_KEY_D)) hex_angular_velocity = forward * -2.5f;
		
		// Movement
		hex_bearing += dt * hex_angular_velocity;
		glm::mat4 hex_rotation = glm::rotate(glm::mat4(1.0f), hex_bearing, glm::vec3(.0f, .0f, 1.0f));
		hex_position += dt * hex_speed * hex_rotation * glm::vec4(1.0f,.0f,.0f,.0f);
		glm::mat4 hex_transform = glm::translate(glm::mat4(1.0f), hex_position) * hex_rotation;

		//
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		Zahra::RenderCommand::SetClearColour({ .01f, 0.005f, .05f, 1.0f });
		Zahra::RenderCommand::Clear();

		Zahra::Renderer::BeginScene(m_Camera);

		Zahra::Renderer::Submit(m_Shader, m_VertexArray, hex_transform);

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
	}

	private:
	std::shared_ptr<Zahra::Shader> m_Shader;
	std::shared_ptr<Zahra::VertexArray> m_VertexArray;
	Zahra::OrthographicCamera m_Camera;

	glm::vec3 camera_velocity = glm::vec3(.0f);

	glm::vec3 hex_position = glm::vec3(.0f);
	int forward = 1;
	float hex_bearing = .0f;
	float hex_speed = .0f;
	float hex_angular_velocity = .0f;
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