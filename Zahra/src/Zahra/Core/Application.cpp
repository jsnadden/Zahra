#include "zpch.h"
#include "Application.h"
#include "Zahra/Core/Input.h"

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
		// TEMPORARILY DOING RENDERING HERE
		// 
		m_VertexArray.reset(VertexArray::Create());

		float vertices[6 * 7] = {
			 0.56f,  0.00f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.28f,  0.87f, 0.0f, 0.7f, 0.7f, 0.0f, 1.0f,
			-0.28f,  0.87f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			-0.56f,  0.00f, 0.0f, 0.0f, 0.7f, 0.7f, 1.0f,
			-0.28f, -0.87f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			 0.28f, -0.87f, 0.0f, 0.7f, 0.0f, 0.7f, 1.0f
		};
		std::shared_ptr<VertexBuffer> vertexBuffer;
		vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

		{
			BufferLayout layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float4, "a_Colour" }
			};

			vertexBuffer->SetLayout(layout);
		}

		m_VertexArray->AddVertexBuffer(vertexBuffer);

		unsigned int indices[12] = { 0,1,2,0,2,3,0,3,4,0,4,5 };
		std::shared_ptr<IndexBuffer> indexBuffer;
		indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);
		
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
			// TEMPORARILY DOING RENDERING HERE
			//
			RenderCommand::SetClearColour({ 1.0f, 0.0f, 1.0f, 1.0f });
			RenderCommand::Clear();
			//
			Renderer::BeginScene();

			m_Shader->Bind();
			Renderer::Submit(m_VertexArray);

			Renderer::EndScene();
			//Renderer::Flush();
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