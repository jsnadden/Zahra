#include "zpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Buffer.h"
#include "Shader.h"
#include "RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Zahra
{
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Colour;
		glm::vec2 TextureCoord;
		//float TextureID;
	};


	struct Renderer2DData
	{
		const uint32_t MaxQuadsPerBuffer = 10000;
		const uint32_t MaxVerticesPerBuffer = MaxQuadsPerBuffer * 4;
		const uint32_t MaxIndicesPerBuffer = MaxQuadsPerBuffer * 6;

		uint32_t QuadIndexCount = 0;

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;

		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;
	};


	static Renderer2DData s_Data;

	void Renderer2D::Init()
	{
		Z_PROFILE_FUNCTION();

		// VERTEX ARRAY
		s_Data.QuadVertexArray = VertexArray::Create();
		{
			// VERTEX BUFFER
			s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVerticesPerBuffer * sizeof(QuadVertex));
			s_Data.QuadVertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position"},
				{ ShaderDataType::Float4, "a_Colour"},
				{ ShaderDataType::Float2, "a_TextureCoord"}
			});
			s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

			s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVerticesPerBuffer];

			// INDEX BUFFER
			uint32_t* quadIndices = new uint32_t[s_Data.MaxIndicesPerBuffer];

			uint32_t offset = 0;
			for (uint32_t i = 0; i < s_Data.MaxIndicesPerBuffer; i += 6)
			{
				quadIndices[i + 0] = offset + 0;
				quadIndices[i + 1] = offset + 1;
				quadIndices[i + 2] = offset + 2;

				quadIndices[i + 3] = offset + 2;
				quadIndices[i + 4] = offset + 3;
				quadIndices[i + 5] = offset + 0;

				offset += 4;
			}

			Ref<IndexBuffer> quadIndexBuffer = IndexBuffer::Create(quadIndices, s_Data.MaxIndicesPerBuffer);
			s_Data.QuadVertexArray->SetIndexBuffer(quadIndexBuffer);
			delete[] quadIndices; // TODO: later we'll be adding these to a queue, in which case they'll need a dynamic lifetime, ideally managed by a reference counting system
		}

		// DEFAULT TEXTURE
		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t flatWhite = 0xffffffff;
		s_Data.WhiteTexture->SetData(&flatWhite, sizeof(uint32_t));

		// SHADER
		s_Data.TextureShader = Shader::Create("C:/dev/Zahra/Zahra/src/Zahra/Renderer/TEMPORARYshaders/texture.glsl");
		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetInt("u_Texture", 0);
	}

	void Renderer2D::Shutdown()
	{
		Z_PROFILE_FUNCTION();


	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		Z_PROFILE_FUNCTION();

		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_PVMatrix", camera.GetPVMatrix());

		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
	}

	void Renderer2D::EndScene()
	{
		Z_PROFILE_FUNCTION();

		uint32_t dataSize = (uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase;
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer2D::Flush()
	{
		Z_PROFILE_FUNCTION();

		RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& colour)
	{
		Z_PROFILE_FUNCTION();

		s_Data.QuadVertexBufferPtr->Position = position;
		s_Data.QuadVertexBufferPtr->Colour = colour;
		s_Data.QuadVertexBufferPtr->TextureCoord = { .0f, .0f };

		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = position + glm::vec3(dimensions.x, .0f, .0f);
		s_Data.QuadVertexBufferPtr->Colour = colour;
		s_Data.QuadVertexBufferPtr->TextureCoord = { 1.0f, .0f };

		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = position + glm::vec3(dimensions.x, dimensions.y, .0f);
		s_Data.QuadVertexBufferPtr->Colour = colour;
		s_Data.QuadVertexBufferPtr->TextureCoord = { 1.0f, 1.0f };

		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = position + glm::vec3(.0f, dimensions.y, .0f);
		s_Data.QuadVertexBufferPtr->Colour = colour;
		s_Data.QuadVertexBufferPtr->TextureCoord = { .0f, 1.0f };

		s_Data.QuadVertexBufferPtr++;
		s_Data.QuadIndexCount += 6;


		/*glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), glm::vec3(dimensions, 1.0f));

		s_Data.TextureShader->SetMat4("u_Transform", transform);
		s_Data.TextureShader->SetFloat("u_Tiling", 1.0f);

		s_Data.WhiteTexture->Bind();

		s_Data.QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray);*/
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& dimensions, const glm::vec4& colour)
	{
		DrawQuad(glm::vec3(position, 0.0f), dimensions, colour);
	}
	


	



}


