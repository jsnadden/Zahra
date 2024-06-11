#include "zpch.h"
#include "Application.h"
#include "Zahra/Input.h"

#include <glad/glad.h>

namespace Zahra
{
	Application* Application::s_Instance = nullptr;


	Application::Application()
	{
		Z_CORE_ASSERT(!s_Instance, "Application already exists");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(Z_BIND_EVENT_FN(Application::OnEvent));

		m_ImGuiLayer = new ImGuiLayer;
		PushOverlay(m_ImGuiLayer);

		/////////////////////////////////////////////////////////////
		// TODO: ABSTRACT THIS STUFF
		// 
		m_VertexArray.reset(VertexArray::Create());

		// Create vertices and send to buffer
		float vertices[6 * 7] = {
			 0.56f,  0.00f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.28f,  0.87f, 0.0f, 0.9f, 0.9f, 0.0f, 1.0f,
			-0.28f,  0.87f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			-0.56f,  0.00f, 0.0f, 0.0f, 0.9f, 0.9f, 1.0f,
			-0.28f, -0.87f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			 0.28f, -0.87f, 0.0f, 0.9f, 0.0f, 0.9f, 1.0f
		};
		m_VertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

		{
			BufferLayout layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float4, "a_Colour" }
			};

			m_VertexBuffer->SetLayout(layout);
		}

		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		// Create indices and send to index buffer (essentially an enumeration of the vertices, and their draw order)
		unsigned int indices[6] = { 0, 1, 2, 3, 4, 5 };
		m_IndexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);
		
		// shader source code
		std::string vertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Colour;

			out vec3 v_Position;
			out vec4 v_Colour;

			void main()
			{
				v_Position = a_Position+0.5;
				v_Colour = a_Colour;
				gl_Position = vec4(a_Position, 1.0);
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

		// compile/link shaders
		m_Shader.reset(new Shader(vertexSrc, fragmentSrc));
		//
		/////////////////////////////////////////////////////////////

	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		while (m_Running)
		{
			/////////////////////////////////////////////////////////////
			// 
			// clear buffers (basically a fresh canvas)
			glClearColor(0.0f, 0.0f, 0.0f, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			// draw!
			m_Shader->Bind();
			m_VertexArray->Bind();
			glDrawElements(GL_TRIANGLE_FAN, m_IndexBuffer->GetCount(), GL_UNSIGNED_INT, NULL);
			//
			/////////////////////////////////////////////////////////////


			// Update layers
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();
			
			// Render ImGui layers
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			// Update window context
			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(Z_BIND_EVENT_FN(Application::OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);
			if (e.Handled) break;
		}

	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		
		return true;
	}


}