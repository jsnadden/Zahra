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
		float TextureIndex;
		float TilingFactor;
	};


	struct Renderer2DData
	{
		const uint32_t MaxQuadsPerBuffer = 10000;
		const uint32_t MaxVerticesPerBuffer = MaxQuadsPerBuffer * 4;
		const uint32_t MaxIndicesPerBuffer = MaxQuadsPerBuffer * 6;
		static const uint32_t MaxTextureSlots = 32; // TODO: query the actual value via the graphics API

		uint32_t QuadIndexCount = 0;

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> TextureShader;

		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // start at 1, because slot 0 will be our default WhiteTexture
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
				{ ShaderDataType::Float2, "a_TextureCoord"},
				{ ShaderDataType::Float, "a_TextureIndex"},
				{ ShaderDataType::Float, "a_TilingFactor"}

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

		// DEFAULT (1X1 WHITE) TEXTURE
		s_Data.TextureSlots[0] = Texture2D::Create(1, 1);
		uint32_t flatWhite = 0xffffffff;
		s_Data.TextureSlots[0]->SetData(&flatWhite, sizeof(uint32_t));

		// SHADER
		s_Data.TextureShader = Shader::Create("C:/dev/Zahra/Zahra/src/Zahra/Renderer/TEMPORARYshaders/texture.glsl");
		s_Data.TextureShader->Bind();

		int textureSamplers[s_Data.MaxTextureSlots];
		for (int i = 0; i < s_Data.MaxTextureSlots; i++) textureSamplers[i] = i;

		s_Data.TextureShader->SetIntArray("u_Textures", s_Data.MaxTextureSlots, textureSamplers);


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

		s_Data.TextureSlotIndex = 1;
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

		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			s_Data.TextureSlots[i]->Bind(i);

		RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Rendering primitives

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& colour)
	{
		Z_PROFILE_FUNCTION();

		s_Data.QuadVertexBufferPtr->Position = position;
		s_Data.QuadVertexBufferPtr->Colour = colour;
		s_Data.QuadVertexBufferPtr->TextureCoord = { 0.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TextureIndex = 0.0f;
		s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;

		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = position + glm::vec3(dimensions.x, 0.0f, 0.0f);
		s_Data.QuadVertexBufferPtr->Colour = colour;
		s_Data.QuadVertexBufferPtr->TextureCoord = { 1.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TextureIndex = 0.0f;
		s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;

		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = position + glm::vec3(dimensions.x, dimensions.y, 0.0f);
		s_Data.QuadVertexBufferPtr->Colour = colour;
		s_Data.QuadVertexBufferPtr->TextureCoord = { 1.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TextureIndex = 0.0f;
		s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;

		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = position + glm::vec3(0.0f, dimensions.y, 0.0f);
		s_Data.QuadVertexBufferPtr->Colour = colour;
		s_Data.QuadVertexBufferPtr->TextureCoord = { 0.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TextureIndex = 0.0f;
		s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;

		s_Data.QuadVertexBufferPtr++;
		s_Data.QuadIndexCount += 6;

	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& dimensions, const glm::vec4& colour)
	{
		DrawQuad(glm::vec3(position, 0.0f), dimensions, colour);
	}
	
	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& dimensions, const Ref<Texture2D> texture, const glm::vec4& tint, float tiling)
	{
		Z_PROFILE_FUNCTION();

		float textureIndex = 0.0f;

		// FIND AN AVAILABLE TEXTURE SLOT
		{

			for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
			{
				// TODO: this is horrendous, refactor it once we have a general asset UUID system
				if (*s_Data.TextureSlots[i].get() == *texture.get())
				{
					textureIndex = (float)i;
					break;
				}
			}

			if (textureIndex == 0.0f)
			{
				textureIndex = (float)s_Data.TextureSlotIndex;
				s_Data.TextureSlotIndex++;
			}

		}

		s_Data.TextureSlots[textureIndex] = texture;

		s_Data.QuadVertexBufferPtr->Position = position;
		s_Data.QuadVertexBufferPtr->Colour = tint;
		s_Data.QuadVertexBufferPtr->TextureCoord = { 0.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tiling;

		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = position + glm::vec3(dimensions.x, 0.0f, 0.0f);
		s_Data.QuadVertexBufferPtr->Colour = tint;
		s_Data.QuadVertexBufferPtr->TextureCoord = { 1.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tiling;

		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = position + glm::vec3(dimensions.x, dimensions.y, 0.0f);
		s_Data.QuadVertexBufferPtr->Colour = tint;
		s_Data.QuadVertexBufferPtr->TextureCoord = { 1.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tiling;

		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = position + glm::vec3(0.0f, dimensions.y, 0.0f);
		s_Data.QuadVertexBufferPtr->Colour = tint;
		s_Data.QuadVertexBufferPtr->TextureCoord = { 0.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tiling;

		s_Data.QuadVertexBufferPtr++;
		s_Data.QuadIndexCount += 6;


	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& dimensions, const Ref<Texture2D> texture, const glm::vec4& tint, float tiling)
	{
		DrawQuad(glm::vec3(position, 0.0f), dimensions, texture, tint, tiling);
	}

	



}


