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
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Tint;
		glm::vec2 TextureCoord;
		float TextureIndex;
		float TilingFactor;
		int EntityID = -1;
	};

	struct CircleVertex
	{
		glm::vec3 WorldPosition;
		glm::vec2 LocalPosition;
		glm::vec4 Colour;
		float Thickness;
		float Fade;
		int EntityID = -1;
	};

	struct LineVertex
	{
		glm::vec3 Position;
		glm::vec4 Colour;
	};

	struct RendererData
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// RENDERER PARAMETERS

		static const uint32_t MaxQuadsPerBuffer = 10000;
		static const uint32_t MaxVerticesPerBuffer = MaxQuadsPerBuffer * 4;
		static const uint32_t MaxIndicesPerBuffer = MaxQuadsPerBuffer * 6;
		static const uint32_t MaxTextureSlots = 32; // TODO: query the actual value via the graphics API

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// QUADS

		Ref<VertexArray>	QuadVertexArray;
		Ref<VertexBuffer>	QuadVertexBuffer;
		Ref<Shader>			QuadShader;

		uint32_t			QuadIndexCount = 0;
		QuadVertex*			QuadVertexBufferBase = nullptr;
		QuadVertex*			QuadVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // start at 1, because slot 0 will be our default WhiteTexture

		glm::vec4 QuadVertexPositions[4];
		glm::vec2 QuadTextureCoords[4];

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLES

		Ref<VertexArray>	CircleVertexArray;
		Ref<VertexBuffer>	CircleVertexBuffer;
		Ref<Shader>			CircleShader;

		uint32_t			CircleIndexCount = 0;
		CircleVertex*		CircleVertexBufferBase = nullptr;
		CircleVertex*		CircleVertexBufferPtr = nullptr;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// LINES

		Ref<VertexArray>	LineVertexArray;
		Ref<VertexBuffer>	LineVertexBuffer;
		Ref<Shader>			LineShader;

		uint32_t			LineVertexCount = 0;
		LineVertex*			LineVertexBufferBase = nullptr;
		LineVertex*			LineVertexBufferPtr = nullptr;

		float LineThickness = 2.f;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA

		struct CameraData
		{
			glm::mat4 ViewProjection;
		};

		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// MISC.

		Renderer::Statistics Stats;
	};

	static RendererData s_Data;


	void Renderer::Init()
	{
		RenderCommand::Init();
		
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//	QUAD VERTEX ARRAY
		s_Data.QuadVertexArray = VertexArray::Create();
		
		// VERTEX BUFFER
		s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVerticesPerBuffer * sizeof(QuadVertex));
		s_Data.QuadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"	   },
			{ ShaderDataType::Float4, "a_Colour"	   },
			{ ShaderDataType::Float2, "a_TextureCoord" },
			{ ShaderDataType::Float,  "a_TextureIndex" },
			{ ShaderDataType::Float,  "a_TilingFactor" },
			{ ShaderDataType::Int,	  "a_EntityID"	   }
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
		
		// TODO: later we'll be adding these to a queue, in which case they'll need a dynamic lifetime, ideally managed by a reference counting system
		delete[] quadIndices;

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//	CIRCLE VERTEX ARRAY
		s_Data.CircleVertexArray = VertexArray::Create();
		
		// VERTEX BUFFER
		s_Data.CircleVertexBuffer = VertexBuffer::Create(s_Data.MaxVerticesPerBuffer * sizeof(CircleVertex));
		s_Data.CircleVertexBuffer->SetLayout({
			{ ShaderDataType::Float3,	"a_WorldPosition"	},
			{ ShaderDataType::Float2,	"a_LocalPosition"	},
			{ ShaderDataType::Float4,	"a_Colour"			},
			{ ShaderDataType::Float,	"a_Thickness"		},
			{ ShaderDataType::Float,	"a_Fade"			},
			{ ShaderDataType::Int,		"a_EntityID"		}
			});
		s_Data.CircleVertexArray->AddVertexBuffer(s_Data.CircleVertexBuffer);

		s_Data.CircleVertexBufferBase = new CircleVertex[s_Data.MaxVerticesPerBuffer];

		// INDEX BUFFER (reused from quads)
		s_Data.CircleVertexArray->SetIndexBuffer(quadIndexBuffer);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//	LINE VERTEX BUFFER/ARRAY
		s_Data.LineVertexArray = VertexArray::Create();

		s_Data.LineVertexBuffer = VertexBuffer::Create(s_Data.MaxVerticesPerBuffer * sizeof(LineVertex));
		s_Data.LineVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Colour"   }
			});
		s_Data.LineVertexArray->AddVertexBuffer(s_Data.LineVertexBuffer);
		s_Data.LineVertexBufferBase = new LineVertex[s_Data.MaxVerticesPerBuffer];

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TEXTURES
		s_Data.TextureSlots[0] = Texture2D::Create(1, 1);
		uint32_t flatWhite = 0xffffffff;
		s_Data.TextureSlots[0]->SetData(&flatWhite, sizeof(uint32_t));

		int textureSamplers[s_Data.MaxTextureSlots];
		for (int i = 0; i < s_Data.MaxTextureSlots; i++) textureSamplers[i] = i;


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// SHADERS

		// TODO: free ourselves from hardcoded shaders?
		s_Data.QuadShader = Shader::Create("Resources/Shaders/renderer_quad.glsl");
		s_Data.CircleShader = Shader::Create("Resources/Shaders/renderer_circle.glsl");
		s_Data.LineShader = Shader::Create("Resources/Shaders/renderer_line.glsl");


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

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	float Renderer::GetLineThickness()
	{
		return s_Data.LineThickness;
	}

	void Renderer::SetLineThickness(float thickness)
	{
		s_Data.LineThickness = thickness;
	}

	void Renderer::SubmitBatch()
	{
		if (s_Data.QuadIndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
			s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

			// Bind textures
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
				s_Data.TextureSlots[i]->Bind(i);

			s_Data.QuadShader->Bind();
			RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
			s_Data.Stats.DrawCalls++;
		}

		if (s_Data.CircleIndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CircleVertexBufferPtr - (uint8_t*)s_Data.CircleVertexBufferBase);
			s_Data.CircleVertexBuffer->SetData(s_Data.CircleVertexBufferBase, dataSize);

			s_Data.CircleShader->Bind();
			RenderCommand::DrawIndexed(s_Data.CircleVertexArray, s_Data.CircleIndexCount);
			s_Data.Stats.DrawCalls++;
		}

		if (s_Data.LineVertexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.LineVertexBufferPtr - (uint8_t*)s_Data.LineVertexBufferBase);
			s_Data.LineVertexBuffer->SetData(s_Data.LineVertexBufferBase, dataSize);

			s_Data.LineShader->Bind();
			RenderCommand::SetLineThickness(s_Data.LineThickness);
			RenderCommand::DrawLines(s_Data.LineVertexArray, s_Data.LineVertexCount);
			s_Data.Stats.DrawCalls++;
		}
	}

	void Renderer::NewBatch()
	{
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.CircleIndexCount = 0;
		s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;

		s_Data.LineVertexCount = 0;
		s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;

		s_Data.TextureSlotIndex = 1;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// PRIMITIVES

	void Renderer::DrawQuad(const glm::mat4& transform, const glm::vec4& colour, int EntityID)
	{
		// TODO: separate maxima for each primitive type
		if (s_Data.QuadIndexCount >= RendererData::MaxIndicesPerBuffer)
		{
			SubmitBatch();
			NewBatch();
		}

		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Tint = colour;
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
		// TODO: separate maxima for each primitive type
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
			s_Data.QuadVertexBufferPtr->Tint = tint;
			s_Data.QuadVertexBufferPtr->TextureCoord = s_Data.QuadTextureCoords[i];
			s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tiling;
			s_Data.QuadVertexBufferPtr->EntityID = EntityID;

			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;
		s_Data.Stats.QuadCount++;
	}

	void Renderer::DrawCircle(const glm::mat4& transform, const glm::vec4& colour, float thickness, float fade, int EntityID)
	{
		// TODO: separate maxima for each primitive type
		if (s_Data.CircleIndexCount >= RendererData::MaxIndicesPerBuffer)
		{
			SubmitBatch();
			NewBatch();
		}

		for (int i = 0; i < 4; i++)
		{
			s_Data.CircleVertexBufferPtr->WorldPosition = transform * s_Data.QuadVertexPositions[i];
			s_Data.CircleVertexBufferPtr->LocalPosition = s_Data.QuadVertexPositions[i] * 2.0f;
			s_Data.CircleVertexBufferPtr->Colour = colour;
			s_Data.CircleVertexBufferPtr->Thickness = thickness;
			s_Data.CircleVertexBufferPtr->Fade = fade;
			s_Data.CircleVertexBufferPtr->EntityID = EntityID;

			s_Data.CircleVertexBufferPtr++;
		}

		s_Data.CircleIndexCount += 6;
		s_Data.Stats.QuadCount++;
	}

	void Renderer::DrawLine(const glm::vec3& end0, const glm::vec3& end1, const glm::vec4& colour)
	{
		// TODO: separate maxima for each primitive type
		if (s_Data.LineVertexCount >= RendererData::MaxVerticesPerBuffer)
		{
			SubmitBatch();
			NewBatch();
		}

		s_Data.LineVertexBufferPtr->Position = end0;
		s_Data.LineVertexBufferPtr->Colour = colour;
		s_Data.LineVertexBufferPtr++;

		s_Data.LineVertexBufferPtr->Position = end1;
		s_Data.LineVertexBufferPtr->Colour = colour;
		s_Data.LineVertexBufferPtr++;

		s_Data.LineVertexCount += 2;
	}

	void Renderer::DrawRect(const glm::mat4& transform, const glm::vec4& colour)
	{
		glm::vec3 corners[4] = { {.5f, .5f, .0f}, {-.5f, .5f, .0f}, {-.5f, -.5f, .0f}, {.5f, -.5f, .0f} };

		for (int i = 0; i < 4; i++)
			DrawLine(transform * glm::vec4(corners[i], 1.f), transform * glm::vec4(corners[(i+1)%4], 1.f), colour);
	}



	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// SPRITES

	void Renderer::DrawSprite(const glm::mat4& transform, SpriteComponent& sprite, int EntityID)
	{
		// TODO: deal with animation data
		if (sprite.Texture)
			DrawQuad(transform, sprite.Texture, sprite.Tint, sprite.TextureTiling, EntityID);
		else
			DrawQuad(transform, sprite.Tint, EntityID);
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
