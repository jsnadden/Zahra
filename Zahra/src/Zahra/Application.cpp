#include "zpch.h"
#include "Application.h"
#include "Zahra/Input.h"

#include <glad/glad.h>

namespace Zahra
{
	Application* Application::s_Instance = nullptr;

	// TODO: move this somewhere more appropriate:
	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case Zahra::ShaderDataType::Bool:   return GL_BOOL;

		case Zahra::ShaderDataType::Int:    return GL_INT;
		case Zahra::ShaderDataType::Int2:   return GL_INT;
		case Zahra::ShaderDataType::Int3:   return GL_INT;
		case Zahra::ShaderDataType::Int4:   return GL_INT;

		case Zahra::ShaderDataType::Float:  return GL_FLOAT;
		case Zahra::ShaderDataType::Float2: return GL_FLOAT;
		case Zahra::ShaderDataType::Float3: return GL_FLOAT;
		case Zahra::ShaderDataType::Float4: return GL_FLOAT;

		case Zahra::ShaderDataType::Mat2:   return GL_FLOAT;
		case Zahra::ShaderDataType::Mat3:   return GL_FLOAT;
		case Zahra::ShaderDataType::Mat4:   return GL_FLOAT;
		}

		Z_CORE_ASSERT(false, "Invalid ShaderDataType");
		return 0;
	}

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
		// Create vertex array on GPU
		glGenVertexArrays(1, &m_VertexArray);
		glBindVertexArray(m_VertexArray);

		// Create vertices and send to buffer
		float vertices[3 * 3] = {
			-0.5f, -0.5f, 0.0f, //1.0f, 0.0f, 1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, //1.0f, 0.0f, 1.0f, 1.0f,
			 0.0f,  0.5f, 0.0f//, 1.0f, 0.0f, 1.0f, 1.0f
		};
		m_VertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" }//,
			//{ ShaderDataType::Float4, "a_Colour" }
		};

		m_VertexBuffer->SetLayout(layout);

		uint32_t index = 0;

		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(
				index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalised ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset
			);
		}

		// Create indices and send to index buffer (essentially an enumeration of the vertices, and their draw order)
		unsigned int indices[3] = { 0, 1, 2 };
		m_IndexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		
		// shader source code
		std::string vertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position+0.5;
				gl_Position = vec4(a_Position, 1.0);
			}
		)";
		std::string fragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 colour;
			
			in vec3 v_Position;

			void main()
			{
				colour = vec4(v_Position, 1.0);
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
			// TODO: ABSTRACT THIS STUFF
			// 
			// clear buffers (basically a fresh canvas)
			glClearColor(0.0f, 0.0f, 0.0f, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			// bind shader and vertices
			m_Shader->Bind();
			glBindVertexArray(m_VertexArray);

			// draw!!
			glDrawElements(GL_TRIANGLES, m_IndexBuffer->GetCount(), GL_UNSIGNED_INT, NULL);
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