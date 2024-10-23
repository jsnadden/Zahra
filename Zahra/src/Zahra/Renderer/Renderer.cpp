#include "zpch.h"
#include "Renderer.h"

#include "Zahra/Renderer/Buffer.h"
#include "Zahra/Renderer/Pipeline.h"
#include "Zahra/Renderer/Shader.h"
#include "Zahra/Renderer/UniformBuffer.h"
#include "Zahra/Renderer/VertexArray.h"

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
		int EntityID = -1;
	};

	struct RendererData
	{
		Ref<Shader> Shader;

		RendererConfig Config;

		// TODO: set pipelines externally?
		Ref<Pipeline> Pipeline;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// PARAMETERS
		//static const uint32_t MaxQuadsPerBuffer = 10000;
		//static const uint32_t MaxVerticesPerBuffer = MaxQuadsPerBuffer * 4;
		//static const uint32_t MaxIndicesPerBuffer = MaxQuadsPerBuffer * 6;
		//static const uint32_t MaxTextureSlots = 32; // TODO: query the actual value via the graphics API

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// QUADS
		//Ref<VertexArray>	QuadVertexArray;
		//Ref<VertexBuffer>	QuadVertexBuffer;
		//Ref<Shader>			QuadShader;

		//uint32_t			QuadIndexCount = 0;
		//QuadVertex*			QuadVertexBufferBase = nullptr;
		//QuadVertex*			QuadVertexBufferPtr = nullptr;

		//std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		//uint32_t TextureSlotIndex = 1; // start at 1, because slot 0 will be our default WhiteTexture

		//glm::vec4 QuadVertexPositions[4];
		//glm::vec2 QuadTextureCoords[4];

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLES
		//Ref<VertexArray>	CircleVertexArray;
		//Ref<VertexBuffer>	CircleVertexBuffer;
		//Ref<Shader>			CircleShader;

		//uint32_t			CircleIndexCount = 0;
		//CircleVertex*		CircleVertexBufferBase = nullptr;
		//CircleVertex*		CircleVertexBufferPtr = nullptr;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// LINES
		//Ref<VertexArray>	LineVertexArray;
		//Ref<VertexBuffer>	LineVertexBuffer;
		//Ref<Shader>			LineShader;

		//uint32_t			LineVertexCount = 0;
		//LineVertex*			LineVertexBufferBase = nullptr;
		//LineVertex*			LineVertexBufferPtr = nullptr;

		//float LineThickness = 2.f;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA
		//struct CameraData
		//{
		//	glm::mat4 ViewProjection;
		//};

		//CameraData CameraBuffer;
		//Ref<UniformBuffer> CameraUniformBuffer;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// STATS
		Renderer::Statistics Stats;
	};

	static RendererData s_RendererData;

	static RendererAPI* s_RendererAPI;

	void Renderer::Init()
	{
		s_RendererAPI = RendererAPI::Create();
		s_RendererAPI->Init();

		// TEMPORARY
		ShaderSpecification shaderSpec{};
		// TODO: the app should send the shader details prior to this (in Renderer::Init() e.g.)
		// (and they should really be owned by a shader library)
		shaderSpec.Name = "vulkan_tutorial";
		shaderSpec.SourceDirectory = "Resources/Shaders";
		s_RendererData.Shader = Shader::Create(shaderSpec);

		PipelineSpecification pipelineSpec{};
		pipelineSpec.Shader = s_RendererData.Shader;
		s_RendererData.Pipeline = Pipeline::Create(pipelineSpec);

		// TODO: ressurect these bits
		#pragma region
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//	QUAD VERTEX ARRAY
		//s_RendererData.QuadVertexArray = VertexArray::Create();
		//
		//// VERTEX BUFFER
		//s_RendererData.QuadVertexBuffer = VertexBuffer::Create(s_RendererData.MaxVerticesPerBuffer * sizeof(QuadVertex));
		//s_RendererData.QuadVertexBuffer->SetLayout({
		//	{ ShaderDataType::Float3, "a_Position"	   },
		//	{ ShaderDataType::Float4, "a_Colour"	   },
		//	{ ShaderDataType::Float2, "a_TextureCoord" },
		//	{ ShaderDataType::Float,  "a_TextureIndex" },
		//	{ ShaderDataType::Float,  "a_TilingFactor" },
		//	{ ShaderDataType::Int,	  "a_EntityID"	   }
		//	});
		//s_RendererData.QuadVertexArray->AddVertexBuffer(s_RendererData.QuadVertexBuffer);

		//s_RendererData.QuadVertexBufferBase = new QuadVertex[s_RendererData.MaxVerticesPerBuffer];

		//// INDEX BUFFER
		//uint32_t* quadIndices = new uint32_t[s_RendererData.MaxIndicesPerBuffer];

		//uint32_t offset = 0;
		//for (uint32_t i = 0; i < s_RendererData.MaxIndicesPerBuffer; i += 6)
		//{
		//	quadIndices[i + 0] = offset + 0;
		//	quadIndices[i + 1] = offset + 1;
		//	quadIndices[i + 2] = offset + 2;

		//	quadIndices[i + 3] = offset + 2;
		//	quadIndices[i + 4] = offset + 3;
		//	quadIndices[i + 5] = offset + 0;

		//	offset += 4;
		//}

		//Ref<IndexBuffer> quadIndexBuffer = IndexBuffer::Create(quadIndices, s_RendererData.MaxIndicesPerBuffer);
		//s_RendererData.QuadVertexArray->SetIndexBuffer(quadIndexBuffer);
		//
		//// TODO: later we'll be adding these to a queue, in which case they'll need a dynamic lifetime, ideally managed by a reference counting system
		//delete[] quadIndices;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////	CIRCLE VERTEX ARRAY
		//s_RendererData.CircleVertexArray = VertexArray::Create();
		//
		//// VERTEX BUFFER
		//s_RendererData.CircleVertexBuffer = VertexBuffer::Create(s_RendererData.MaxVerticesPerBuffer * sizeof(CircleVertex));
		//s_RendererData.CircleVertexBuffer->SetLayout({
		//	{ ShaderDataType::Float3,	"a_WorldPosition"	},
		//	{ ShaderDataType::Float2,	"a_LocalPosition"	},
		//	{ ShaderDataType::Float4,	"a_Colour"			},
		//	{ ShaderDataType::Float,	"a_Thickness"		},
		//	{ ShaderDataType::Float,	"a_Fade"			},
		//	{ ShaderDataType::Int,		"a_EntityID"		}
		//	});
		//s_RendererData.CircleVertexArray->AddVertexBuffer(s_RendererData.CircleVertexBuffer);

		//s_RendererData.CircleVertexBufferBase = new CircleVertex[s_RendererData.MaxVerticesPerBuffer];

		//// INDEX BUFFER (reused from quads)
		//s_RendererData.CircleVertexArray->SetIndexBuffer(quadIndexBuffer);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////	LINE VERTEX BUFFER/ARRAY
		//s_RendererData.LineVertexArray = VertexArray::Create();

		//s_RendererData.LineVertexBuffer = VertexBuffer::Create(s_RendererData.MaxVerticesPerBuffer * sizeof(LineVertex));
		//s_RendererData.LineVertexBuffer->SetLayout({
		//	{ ShaderDataType::Float3, "a_Position" },
		//	{ ShaderDataType::Float4, "a_Colour"   },
		//	{ ShaderDataType::Int,		"a_EntityID"		}
		//	});
		//s_RendererData.LineVertexArray->AddVertexBuffer(s_RendererData.LineVertexBuffer);
		//s_RendererData.LineVertexBufferBase = new LineVertex[s_RendererData.MaxVerticesPerBuffer];

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//// TEXTURES
		//s_RendererData.TextureSlots[0] = Texture2D::Create(1, 1);
		//uint32_t flatWhite = 0xffffffff;
		//s_RendererData.TextureSlots[0]->SetData(&flatWhite, sizeof(uint32_t));

		//int textureSamplers[s_RendererData.MaxTextureSlots];
		//for (int i = 0; i < s_RendererData.MaxTextureSlots; i++) textureSamplers[i] = i;


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//// SHADERS

		//// TODO: free ourselves from hardcoded shaders?
		//s_RendererData.QuadShader = Shader::Create("Resources/Shaders/renderer_quad.glsl");
		//s_RendererData.CircleShader = Shader::Create("Resources/Shaders/renderer_circle.glsl");
		//s_RendererData.LineShader = Shader::Create("Resources/Shaders/renderer_line.glsl");


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//// DEFAULT QUAD
		//s_RendererData.QuadVertexPositions[0] = { -.5f, -.5f, .0f, 1.0f };
		//s_RendererData.QuadVertexPositions[1] = { .5f, -.5f, .0f, 1.0f };
		//s_RendererData.QuadVertexPositions[2] = { .5f,  .5f, .0f, 1.0f };
		//s_RendererData.QuadVertexPositions[3] = { -.5f,  .5f, .0f, 1.0f };

		//s_RendererData.QuadTextureCoords[0] = { 0.0f, 0.0f };
		//s_RendererData.QuadTextureCoords[1] = { 1.0f, 0.0f };
		//s_RendererData.QuadTextureCoords[2] = { 1.0f, 1.0f };
		//s_RendererData.QuadTextureCoords[3] = { 0.0f, 1.0f };

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//// CAMERA BUFFER
		//s_RendererData.CameraUniformBuffer = UniformBuffer::Create(sizeof(RendererData::CameraData), 0);
		#pragma endregion
	}

	void Renderer::Shutdown()
	{
		// TEMPORARY
		s_RendererData.Pipeline.Reset();
		s_RendererData.Shader.Reset();

		//delete[] s_RendererData.QuadVertexBufferBase;

		s_RendererAPI->Shutdown();
	}

	RendererConfig& Renderer::GetConfig()
	{
		return s_RendererData.Config;
	}

	void Renderer::SetConfig(const RendererConfig& config)
	{
		s_RendererData.Config = config;
	}

	void Renderer::NewFrame()
	{
		s_RendererAPI->NewFrame();

		// TODO: reset descriptor pools
	}

	void Renderer::DrawTutorialScene()
	{
		s_RendererAPI->BeginRenderPass(s_RendererData.Pipeline);
		s_RendererAPI->TutorialDrawCalls();
		s_RendererAPI->EndRenderPass();
	}

	void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		/*s_RendererData.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_RendererData.CameraUniformBuffer->SetData(&s_RendererData.CameraBuffer, sizeof(RendererData::CameraData));

		NewBatch();*/
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		/*s_RendererData.CameraBuffer.ViewProjection = camera.GetPVMatrix();
		s_RendererData.CameraUniformBuffer->SetData(&s_RendererData.CameraBuffer, sizeof(RendererData::CameraData));

		NewBatch();*/
	}

	void Renderer::EndScene()
	{
		//SubmitBatch();
	}

	void Renderer::PresentImage()
	{
		s_RendererAPI->PresentImage();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		s_RendererAPI->OnWindowResize();
	}

	/*float Renderer::GetLineThickness()
	{
		return s_RendererData.LineThickness;
	}

	void Renderer::SetLineThickness(float thickness)
	{
		//s_RendererData.LineThickness = thickness;
	}*/

	//void Renderer::SubmitBatch()
	//{
	//	if (s_RendererData.QuadIndexCount)
	//	{
	//		uint32_t dataSize = (uint32_t)((uint8_t*)s_RendererData.QuadVertexBufferPtr - (uint8_t*)s_RendererData.QuadVertexBufferBase);
	//		s_RendererData.QuadVertexBuffer->SetData(s_RendererData.QuadVertexBufferBase, dataSize);

	//		// Bind textures
	//		for (uint32_t i = 0; i < s_RendererData.TextureSlotIndex; i++)
	//			s_RendererData.TextureSlots[i]->Bind(i);

	//		s_RendererData.QuadShader->Bind();
	//		//RenderCommandQueue::DrawIndexed(s_RendererData.QuadVertexArray, s_RendererData.QuadIndexCount);
	//		s_RendererData.Stats.DrawCalls++;
	//	}

	//	if (s_RendererData.CircleIndexCount)
	//	{
	//		uint32_t dataSize = (uint32_t)((uint8_t*)s_RendererData.CircleVertexBufferPtr - (uint8_t*)s_RendererData.CircleVertexBufferBase);
	//		s_RendererData.CircleVertexBuffer->SetData(s_RendererData.CircleVertexBufferBase, dataSize);

	//		s_RendererData.CircleShader->Bind();
	//		//RenderCommandQueue::DrawIndexed(s_RendererData.CircleVertexArray, s_RendererData.CircleIndexCount);
	//		s_RendererData.Stats.DrawCalls++;
	//	}

	//	//if (s_RendererData.LineVertexCount)
	//	//{
	//	//	uint32_t dataSize = (uint32_t)((uint8_t*)s_RendererData.LineVertexBufferPtr - (uint8_t*)s_RendererData.LineVertexBufferBase);
	//	//	s_RendererData.LineVertexBuffer->SetData(s_RendererData.LineVertexBufferBase, dataSize);

	//	//	s_RendererData.LineShader->Bind();
	//	//	RenderCommandQueue::SetLineThickness(s_RendererData.LineThickness);
	//	//	//RenderCommandQueue::DrawLines(s_RendererData.LineVertexArray, s_RendererData.LineVertexCount);
	//	//	s_RendererData.Stats.DrawCalls++;
	//	//}
	//}

	//void Renderer::NewBatch()
	//{
	//	s_RendererData.QuadIndexCount = 0;
	//	s_RendererData.QuadVertexBufferPtr = s_RendererData.QuadVertexBufferBase;

	//	s_RendererData.CircleIndexCount = 0;
	//	s_RendererData.CircleVertexBufferPtr = s_RendererData.CircleVertexBufferBase;

	//	s_RendererData.LineVertexCount = 0;
	//	s_RendererData.LineVertexBufferPtr = s_RendererData.LineVertexBufferBase;

	//	s_RendererData.TextureSlotIndex = 1;
	//}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// PRIMITIVES

	void Renderer::DrawQuad(const glm::mat4& transform, const glm::vec4& colour, int entityID)
	{
		//// TODO: separate maxima for each primitive type
		//if (s_RendererData.QuadIndexCount >= RendererData::MaxIndicesPerBuffer)
		//{
		//	SubmitBatch();
		//	NewBatch();
		//}

		//for (int i = 0; i < 4; i++)
		//{
		//	s_RendererData.QuadVertexBufferPtr->Position = transform * s_RendererData.QuadVertexPositions[i];
		//	s_RendererData.QuadVertexBufferPtr->Tint = colour;
		//	s_RendererData.QuadVertexBufferPtr->TextureCoord = s_RendererData.QuadTextureCoords[i];
		//	s_RendererData.QuadVertexBufferPtr->TextureIndex = 0.0f;
		//	s_RendererData.QuadVertexBufferPtr->TilingFactor = 1.0f;
		//	s_RendererData.QuadVertexBufferPtr->EntityID = entityID;

		//	s_RendererData.QuadVertexBufferPtr++;
		//}

		//s_RendererData.QuadIndexCount += 6;
		//s_RendererData.Stats.QuadCount++;
	}

	void Renderer::DrawQuad(const glm::mat4& transform, const Ref<Texture2D> texture, const glm::vec4& tint, float tiling, int entityID)
	{
		//// TODO: separate maxima for each primitive type
		//if (s_RendererData.QuadIndexCount >= RendererData::MaxIndicesPerBuffer)
		//{
		//	SubmitBatch();
		//	NewBatch();
		//}

		//// FIND AN AVAILABLE TEXTURE SLOT
		//float textureIndex = 0.0f;
		//{

		//	for (uint32_t i = 1; i < s_RendererData.TextureSlotIndex; i++)
		//	{
		//		// TODO: this comparison is horrendous, refactor it once we have a general asset UUID system
		//		if (*s_RendererData.TextureSlots[i].Raw() == *texture.Raw())
		//		{
		//			textureIndex = (float)i;
		//			break;
		//		}
		//	}

		//	if (textureIndex == 0.0f)
		//	{
		//		if (s_RendererData.TextureSlotIndex >= RendererData::MaxTextureSlots)
		//		{
		//			SubmitBatch();
		//			NewBatch();
		//		}

		//		textureIndex = (float)s_RendererData.TextureSlotIndex;
		//		s_RendererData.TextureSlots[s_RendererData.TextureSlotIndex] = texture;
		//		s_RendererData.TextureSlotIndex++;
		//	}
		//}

		//for (int i = 0; i < 4; i++)
		//{
		//	s_RendererData.QuadVertexBufferPtr->Position = transform * s_RendererData.QuadVertexPositions[i];
		//	s_RendererData.QuadVertexBufferPtr->Tint = tint;
		//	s_RendererData.QuadVertexBufferPtr->TextureCoord = s_RendererData.QuadTextureCoords[i];
		//	s_RendererData.QuadVertexBufferPtr->TextureIndex = textureIndex;
		//	s_RendererData.QuadVertexBufferPtr->TilingFactor = tiling;
		//	s_RendererData.QuadVertexBufferPtr->EntityID = entityID;

		//	s_RendererData.QuadVertexBufferPtr++;
		//}

		//s_RendererData.QuadIndexCount += 6;
		//s_RendererData.Stats.QuadCount++;
	}

	void Renderer::DrawCircle(const glm::mat4& transform, const glm::vec4& colour, float thickness, float fade, int entityID)
	{
		//// TODO: separate maxima for each primitive type
		//if (s_RendererData.CircleIndexCount >= RendererData::MaxIndicesPerBuffer)
		//{
		//	SubmitBatch();
		//	NewBatch();
		//}

		//for (int i = 0; i < 4; i++)
		//{
		//	s_RendererData.CircleVertexBufferPtr->WorldPosition = transform * s_RendererData.QuadVertexPositions[i];
		//	s_RendererData.CircleVertexBufferPtr->LocalPosition = s_RendererData.QuadVertexPositions[i] * 2.0f;
		//	s_RendererData.CircleVertexBufferPtr->Colour = colour;
		//	s_RendererData.CircleVertexBufferPtr->Thickness = thickness;
		//	s_RendererData.CircleVertexBufferPtr->Fade = fade;
		//	s_RendererData.CircleVertexBufferPtr->EntityID = entityID;

		//	s_RendererData.CircleVertexBufferPtr++;
		//}

		//s_RendererData.CircleIndexCount += 6;
		//s_RendererData.Stats.QuadCount++;
	}

	void Renderer::DrawLine(const glm::vec3& end0, const glm::vec3& end1, const glm::vec4& colour, int entityID)
	{
		//// TODO: separate maxima for each primitive type
		//if (s_RendererData.LineVertexCount >= RendererData::MaxVerticesPerBuffer)
		//{
		//	SubmitBatch();
		//	NewBatch();
		//}

		//s_RendererData.LineVertexBufferPtr->Position = end0;
		//s_RendererData.LineVertexBufferPtr->Colour = colour;
		//s_RendererData.LineVertexBufferPtr->EntityID = entityID;
		//s_RendererData.LineVertexBufferPtr++;

		//s_RendererData.LineVertexBufferPtr->Position = end1;
		//s_RendererData.LineVertexBufferPtr->Colour = colour;
		//s_RendererData.LineVertexBufferPtr->EntityID = entityID;
		//s_RendererData.LineVertexBufferPtr++;

		//s_RendererData.LineVertexCount += 2;
	}

	void Renderer::DrawRect(const glm::mat4& transform, const glm::vec4& colour, int entityID)
	{
		/*glm::vec3 corners[4] = { {.5f, .5f, .0f}, {-.5f, .5f, .0f}, {-.5f, -.5f, .0f}, {.5f, -.5f, .0f} };

		for (int i = 0; i < 4; i++)
			DrawLine(transform * glm::vec4(corners[i], 1.f), transform * glm::vec4(corners[(i+1)%4], 1.f), colour, entityID);*/
	}



	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// SPRITES

	void Renderer::DrawSprite(const glm::mat4& transform, SpriteComponent& sprite, int entityID)
	{
		//// TODO: deal with animation data
		//if (sprite.Texture)
		//	DrawQuad(transform, sprite.Texture, sprite.Tint, sprite.TextureTiling, entityID);
		//else
		//	DrawQuad(transform, sprite.Tint, entityID);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// STATS

	Renderer::Statistics Renderer::GetStats()
	{
		return s_RendererData.Stats;
	}

	void Renderer::ResetStats()
	{
		memset(&s_RendererData.Stats, 0, sizeof(Renderer::Statistics));
	}
	

}
