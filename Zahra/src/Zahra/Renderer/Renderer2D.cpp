#include "zpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Buffer.h"
#include "Shader.h"
#include "RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Zahra
{

	struct Renderer2DData
	{
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;
	};

	static Renderer2DData* s_Data;

	void Renderer2D::Init()
	{
		Z_PROFILE_FUNCTION();

		s_Data = new Renderer2DData;

		// VERTEX ARRAY
		s_Data->QuadVertexArray = VertexArray::Create();
		{
			// VERTEX BUFFER
			float quadVertices[4 * 5] = {
				 0.50f,  0.50f, 0.0f, 1.0f, 1.0f,
				-0.50f,  0.50f, 0.0f, 0.0f, 1.0f,
				-0.50f, -0.50f, 0.0f, 0.0f, 0.0f,
				 0.50f, -0.50f, 0.0f, 1.0f, 0.0f
			};
			Ref<VertexBuffer> quadVertexBuffer = VertexBuffer::Create(quadVertices, sizeof(quadVertices));
			quadVertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position"},
				{ ShaderDataType::Float2, "a_TextureCoord"}
			});
			s_Data->QuadVertexArray->AddVertexBuffer(quadVertexBuffer);

			// INDEX BUFFER
			unsigned int quadIndices[6] = { 0, 1, 2, 0, 2, 3 };
			Ref<IndexBuffer> quadIndexBuffer = IndexBuffer::Create(quadIndices, sizeof(quadIndices) / sizeof(uint32_t));
			s_Data->QuadVertexArray->SetIndexBuffer(quadIndexBuffer);
		}

		// DEFAULT TEXTURE
		s_Data->WhiteTexture = Texture2D::Create(1, 1);
		uint32_t flatWhite = 0xffffffff;
		s_Data->WhiteTexture->SetData(&flatWhite, sizeof(uint32_t));

		// SHADER
		s_Data->TextureShader = Shader::Create("C:/dev/Zahra/Zahra/src/Zahra/Renderer/TEMPORARYshaders/texture.glsl");
		s_Data->TextureShader->Bind();
		s_Data->TextureShader->SetInt("u_Texture", 0);
	}

	void Renderer2D::Shutdown()
	{
		Z_PROFILE_FUNCTION();

		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		Z_PROFILE_FUNCTION();

		s_Data->TextureShader->Bind();
		s_Data->TextureShader->SetMat4("u_PVMatrix", camera.GetPVMatrix());
		
	}

	void Renderer2D::EndScene()
	{
		Z_PROFILE_FUNCTION();


	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& dimensions, const Ref<Texture> texture, const glm::vec4& tint, float tiling)
	{
		Z_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), glm::vec3(dimensions, 1.0f));

		s_Data->TextureShader->SetMat4("u_Transform", transform);
		s_Data->TextureShader->SetFloat4("u_Colour", tint);
		s_Data->TextureShader->SetFloat("u_Tiling", tiling);

		texture->Bind();

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& dimensions, const Ref<Texture> texture, const glm::vec4& tint, float tiling)
	{
		DrawQuad(glm::vec3(position, .0f), dimensions, texture, tint, tiling);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& colour)
	{
		Z_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), glm::vec3(dimensions, 1.0f));

		s_Data->TextureShader->SetMat4("u_Transform", transform);
		s_Data->TextureShader->SetFloat4("u_Colour", colour);
		s_Data->TextureShader->SetFloat("u_Tiling", 1.0f);

		s_Data->WhiteTexture->Bind();

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& dimensions, const glm::vec4& colour)
	{
		DrawQuad(glm::vec3(position, 0.0f), dimensions, colour);
	}
	
	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& dimensions, float rotation, const Ref<Texture> texture, const glm::vec4& tint, float tiling)
	{
		Z_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(.0f, .0f, 1.0f))
			* glm::scale(glm::mat4(1.0f), glm::vec3(dimensions, 1.0f));

		s_Data->TextureShader->SetMat4("u_Transform", transform);
		s_Data->TextureShader->SetFloat4("u_Colour", tint);
		s_Data->TextureShader->SetFloat("u_Tiling", tiling);

		texture->Bind();

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& dimensions, float rotation, const Ref<Texture> texture, const glm::vec4& tint, float tiling)
	{
		DrawRotatedQuad(glm::vec3(position, .0f), dimensions, rotation, texture, tint, tiling);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& dimensions, float rotation, const glm::vec4& colour)
	{
		Z_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(.0f, .0f, 1.0f))
			* glm::scale(glm::mat4(1.0f), glm::vec3(dimensions, 1.0f));

		s_Data->TextureShader->SetMat4("u_Transform", transform);
		s_Data->TextureShader->SetFloat4("u_Colour", colour);
		s_Data->TextureShader->SetFloat("u_Tiling", 1.0f);

		s_Data->WhiteTexture->Bind();

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& dimensions, float rotation, const glm::vec4& colour)
	{
		DrawRotatedQuad(glm::vec3(position, 0.0f), dimensions, rotation, colour);
	}

	



}


