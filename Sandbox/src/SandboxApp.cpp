#include <Zahra.h>


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

			out vec3 v_Position;
			out vec4 v_Colour;

			void main()
			{
				v_Position = a_Position+0.5;
				v_Colour = a_Colour;
				gl_Position = u_PVMatrix * vec4(a_Position, 1.0);
			}
		)";
		std::string fragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 colour;
			
			in vec3 v_Position;
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
		Z_TRACE("Framerate: {0}fps", 1/dt);

		cameravelocity = glm::vec3(0.0f);

		if (Zahra::Input::IsKeyPressed(Z_KEY_W)) cameravelocity.y += 1.0f;
		if (Zahra::Input::IsKeyPressed(Z_KEY_A)) cameravelocity.x -= 1.0f;
		if (Zahra::Input::IsKeyPressed(Z_KEY_S)) cameravelocity.y -= 1.0f;
		if (Zahra::Input::IsKeyPressed(Z_KEY_D)) cameravelocity.x += 1.0f;

		glm::normalize(cameravelocity);

		m_Camera.SetPosition(m_Camera.GetPosition() + dt * cameravelocity);


		Zahra::RenderCommand::SetClearColour({ .01f, 0.005f, .05f, 1.0f });
		Zahra::RenderCommand::Clear();

		Zahra::Renderer::BeginScene(m_Camera);

		Zahra::Renderer::Submit(m_Shader, m_VertexArray);

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

	glm::vec3 cameravelocity = glm::vec3(1,1,0);
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