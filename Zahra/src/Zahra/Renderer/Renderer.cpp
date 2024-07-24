#include "zpch.h"
#include "Renderer.h"

#include "VertexArray.h"
#include "Buffer.h"
#include "UniformBuffer.h"
#include "Shader.h"
#include "RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace Zahra
{

	// TODO: This struct needs to mirror both the bufferlayout and shader inputs, so if
	// I was going to generalise this at all, I'd have to automate constructing all three in parallel
	struct QuadVertexWithEntityID
	{
		glm::vec3 Position;
		glm::vec4 Colour;
		glm::vec2 TextureCoord;
		float TextureIndex;
		float TilingFactor;
		int EntityID = -1;
	};

	struct RendererData
	{
		static const uint32_t MaxQuadsPerBuffer = 10000;
		static const uint32_t MaxVerticesPerBuffer = MaxQuadsPerBuffer * 4;
		static const uint32_t MaxIndicesPerBuffer = MaxQuadsPerBuffer * 6;
		static const uint32_t MaxTextureSlots = 32; // TODO: query the actual value via the graphics API

		uint32_t QuadIndexCount = 0;

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> TextureShader;

		QuadVertexWithEntityID* QuadVertexBufferBase = nullptr;
		QuadVertexWithEntityID* QuadVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // start at 1, because slot 0 will be our default WhiteTexture

		glm::vec4 QuadVertexPositions[4];
		glm::vec2 QuadTextureCoords[4];

		Renderer::Statistics Stats;

		struct CameraData
		{
			glm::mat4 ViewProjection;
		};

		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;
	};

	static RendererData s_Data;



	void Renderer::Init()
	{
		RenderCommand::Init();
		
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// VERTEX ARRAY
		s_Data.QuadVertexArray = VertexArray::Create();
		{
			// VERTEX BUFFER
			s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVerticesPerBuffer * sizeof(QuadVertexWithEntityID));
			s_Data.QuadVertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position"	   },
				{ ShaderDataType::Float4, "a_Colour"	   },
				{ ShaderDataType::Float2, "a_TextureCoord" },
				{ ShaderDataType::Float,  "a_TextureIndex" },
				{ ShaderDataType::Float,  "a_TilingFactor" },
				{ ShaderDataType::Int,	  "a_EntityID"	   }

				});
			s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

			s_Data.QuadVertexBufferBase = new QuadVertexWithEntityID[s_Data.MaxVerticesPerBuffer];

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


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TEXTURES
		s_Data.TextureSlots[0] = Texture2D::Create(1, 1);
		uint32_t flatWhite = 0xffffffff;
		s_Data.TextureSlots[0]->SetData(&flatWhite, sizeof(uint32_t));

		int textureSamplers[s_Data.MaxTextureSlots];
		for (int i = 0; i < s_Data.MaxTextureSlots; i++) textureSamplers[i] = i;


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// SHADERS

		s_Data.TextureShader = Shader::Create("assets/shaders/editor_texture.glsl"); // TODO: free ourselves from a hardcoded shader


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// DEFAULT QUAD
		s_Data.QuadVertexPositions[0] = { -.5f, -.5f, .0f, 1.0f };
		s_Data.QuadVertexPositions[1] = { .5f, -.5f, .0f, 1.0f };
		s_Data.QuadVertexPositions[2] = { .5f,  .5f, .0f, 1.0f };
		s_Data.QuadVertexPositions[3] = { -.5f,  .5f, .0f, 1.0f };

		s_Data.QuadTextureCoords[0] = { 0.0f, 0.0f };
		s_Data.QuadTextureCoords[1] = { 1.0f, 0.0f };
		s_Data.QuadTextureCoords[2] = { 1.0f, 1.0f };
		s_Data.QuadTextureCoords[3] = { 0.0f, 1.0f };

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA BUFFER
		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(RendererData::CameraData), 0);
	}

	void Renderer::Shutdown()
	{
		delete[] s_Data.QuadVertexBufferBase;
	}

	void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		s_Data.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));

		NewBatch();
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		s_Data.CameraBuffer.ViewProjection = camera.GetPVMatrix();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));

		NewBatch();
	}

	void Renderer::EndScene()
	{
		Z_PROFILE_FUNCTION();

		SubmitBatch();
	}

	void Renderer::Flush()
	{
		Z_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount == 0)
			return; // Nothing to draw, early out

		// Bind textures
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			s_Data.TextureSlots[i]->Bind(i);

		s_Data.TextureShader->Bind();
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
		s_Data.Stats.DrawCalls++;
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::SubmitBatch()
	{
		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer::NewBatch()
	{
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.TextureSlotIndex = 1;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// PRIMITIVES
	// TODO: remove the entityIds from drawquad
	void Renderer::DrawQuad(const glm::mat4& transform, const glm::vec4& colour, int EntityID)
	{
		if (s_Data.QuadIndexCount >= RendererData::MaxIndicesPerBuffer)
		{
			SubmitBatch();
			NewBatch();
		}

		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Colour = colour;
			s_Data.QuadVertexBufferPtr->TextureCoord = s_Data.QuadTextureCoords[i];
			s_Data.QuadVertexBufferPtr->TextureIndex = 0.0f;
			s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;
			s_Data.QuadVertexBufferPtr->EntityID = EntityID;

			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;
		s_Data.Stats.QuadCount++;
	}

	void Renderer::DrawQuad(const glm::mat4& transform, const Ref<Texture2D> texture, const glm::vec4& tint, float tiling, int EntityID)
	{
		if (s_Data.QuadIndexCount >= RendererData::MaxIndicesPerBuffer)
		{
			SubmitBatch();
			NewBatch();
		}

		// FIND AN AVAILABLE TEXTURE SLOT
		float textureIndex = 0.0f;
		{

			for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
			{
				// TODO: this comparison is horrendous, refactor it once we have a general asset UUID system
				if (*s_Data.TextureSlots[i].get() == *texture.get())
				{
					textureIndex = (float)i;
					break;
				}
			}

			if (textureIndex == 0.0f)
			{
				if (s_Data.TextureSlotIndex >= RendererData::MaxTextureSlots)
				{
					SubmitBatch();
					NewBatch();
				}

				textureIndex = (float)s_Data.TextureSlotIndex;
				s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
				s_Data.TextureSlotIndex++;
			}
		}

		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Colour = tint;
			s_Data.QuadVertexBufferPtr->TextureCoord = s_Data.QuadTextureCoords[i];
			s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tiling;
			s_Data.QuadVertexBufferPtr->EntityID = EntityID;

			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;
		s_Data.Stats.QuadCount++;
	}

	void Renderer::DrawQuad(const glm::vec3& position, const glm::vec2& dimensions, const glm::vec4& colour)
	{

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { dimensions.x, dimensions.y, 1.0f });

		DrawQuad(transform, colour);

	}

	void Renderer::DrawQuad(const glm::vec2& position, const glm::vec2& dimensions, const glm::vec4& colour)
	{
		DrawQuad(glm::vec3(position, 0.0f), dimensions, colour);
	}

	void Renderer::DrawQuad(const glm::vec3& position, const glm::vec2& dimensions, const Ref<Texture2D> texture, const glm::vec4& tint, float tiling)
	{

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { dimensions.x, dimensions.y, 1.0f });

		DrawQuad(transform, texture, tint, tiling);

	}

	void Renderer::DrawQuad(const glm::vec2& position, const glm::vec2& dimensions, const Ref<Texture2D> texture, const glm::vec4& tint, float tiling)
	{
		DrawQuad(glm::vec3(position, 0.0f), dimensions, texture, tint, tiling);
	}

	void Renderer::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& dimensions, float rotation, const glm::vec4& colour)
	{

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(.0f, .0f, 1.0f))
			* glm::scale(glm::mat4(1.0f), { dimensions.x, dimensions.y, 1.0f });

		DrawQuad(transform, colour);

	}

	void Renderer::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& dimensions, float rotation, const glm::vec4& colour)
	{
		DrawRotatedQuad(glm::vec3(position, 0.0f), dimensions, rotation, colour);
	}

	void Renderer::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& dimensions, float rotation, const Ref<Texture2D> texture, const glm::vec4& tint, float tiling)
	{

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(.0f, .0f, 1.0f))
			* glm::scale(glm::mat4(1.0f), { dimensions.x, dimensions.y, 1.0f });

		DrawQuad(transform, texture, tint, tiling);

	}

	void Renderer::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& dimensions, float rotation, const Ref<Texture2D> texture, const glm::vec4& tint, float tiling)
	{
		DrawRotatedQuad(glm::vec3(position, 0.0f), dimensions, rotation, texture, tint, tiling);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// SPRITES

	void Renderer::DrawSprite(const glm::mat4& transform, SpriteComponent& sprite, int EntityID)
	{
		// TODO: incorporate possibility of textures, animation etc.
		DrawQuad(transform, sprite.Colour, EntityID);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// STATS

	Renderer::Statistics Renderer::GetStats()
	{
		return s_Data.Stats;
	}

	void Renderer::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Renderer::Statistics));
	}
	

}
