#include "zpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Buffer.h"
#include "Shader.h"
#include "RenderCommand.h"

// TODO: remove this dependency
#include "Platform/OpenGL/OpenGLShader.h"

namespace Zahra
{

	struct Renderer2DData
	{
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader> FlatColourShader;
	};

	static Renderer2DData* s_Data;

	void Renderer2D::Init()
	{
		s_Data = new Renderer2DData;

		// VERTEX ARRAY
		s_Data->QuadVertexArray = VertexArray::Create();
		{
			// VERTEX BUFFER
			float squareVertices[4 * 3] = {
				 0.50f,  0.50f, 0.0f,
				-0.50f,  0.50f, 0.0f,
				-0.50f, -0.50f, 0.0f,
				 0.50f, -0.50f, 0.0f
			};
			Ref<VertexBuffer> squareVB = VertexBuffer::Create(squareVertices, sizeof(squareVertices));
			squareVB->SetLayout({ { ShaderDataType::Float3, "a_Position" } });
			s_Data->QuadVertexArray->AddVertexBuffer(squareVB);

			// INDEX BUFFER
			unsigned int squareIndices[6] = { 0, 1, 2, 0, 2, 3 };
			Ref<IndexBuffer> squareIB = IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
			s_Data->QuadVertexArray->SetIndexBuffer(squareIB);
		}

		// SHADER   // TODO: this class is in Zahra, but the shader is an asset in sandbox... EW!
		s_Data->FlatColourShader = Shader::Create("C:/dev/Zahra/Sandbox/assets/shaders/flatcolour.glsl");

	}

	void Renderer2D::Shutdown()
	{
		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColourShader)->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColourShader)->UploadUniformMat4("u_PVMatrix", camera.GetPVMatrix());
		
	}

	void Renderer2D::EndScene()
	{

	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& colour)
	{
		glm::mat4 transform = glm::mat4(1.0f); // TODO get transform from position and dimensions

		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColourShader)->Bind(); 
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColourShader)->UploadUniformMat4("u_Transform", transform);
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColourShader)->UploadUniformFloat4("u_Colour", colour);

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

}


