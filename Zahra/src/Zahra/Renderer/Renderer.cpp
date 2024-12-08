#include "zpch.h"
#include "Renderer.h"

#include "Zahra/Core/Types.h"
#include "Zahra/Renderer/IndexBuffer.h"
#include "Zahra/Renderer/Mesh.h"
#include "Zahra/Renderer/RenderPass.h"
#include "Zahra/Renderer/RendererTypes.h"
#include "Zahra/Renderer/Shader.h"
#include "Zahra/Renderer/ShaderResourceManager.h"
#include "Zahra/Renderer/UniformBuffer.h"
#include "Zahra/Renderer/VertexBuffer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace Zahra
{

	// TEMPORARY
	struct MVPTransforms
	{
		// don't forget to align uniform data correctly: https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets
		alignas(16) glm::mat4 Model;
		alignas(16) glm::mat4 View;
		alignas(16) glm::mat4 Projection;
	};

	struct RendererData
	{
		#pragma region(VULKAN TUTORIAL)
		Ref<Shader> TutorialShader;
		Ref<ShaderResourceManager> TutorialResourceManager;
		Ref<RenderPass> TutorialRenderPass;
		Ref<StaticMesh> TutorialMesh;
		Ref<UniformBufferSet> TutorialUniformBuffers;
		Ref<Texture2D> TutorialTexture;
		#pragma endregion

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CONFIG
		RendererConfig Config;

		// TODO: migrate these into RenderConfig
		static const uint32_t MaxQuadsPerBuffer = 10000;
		static const uint32_t MaxVerticesPerBuffer = MaxQuadsPerBuffer * 4;
		static const uint32_t MaxIndicesPerBuffer = MaxQuadsPerBuffer * 6;
		static const uint32_t MaxTextureSlots = 32;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TEXTURES
		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // start at 1, because slot 0 will be our default 1x1 white texture
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// QUADS
		Ref<Shader> QuadShader;
		Ref<ShaderResourceManager> QuadResourceManager;
		Ref<RenderPass> QuadPass;
		std::vector<Ref<VertexBuffer>>	QuadVertexBuffers; // one per frame-in-flight
		std::vector<QuadVertex*> QuadVertexBufferBases;
		std::vector<QuadVertex*> QuadVertexBufferPtrs;
		glm::vec4 QuadVertexPositions[4];
		glm::vec2 QuadTextureCoords[4];
		Ref<IndexBuffer> QuadIndexBuffer;
		uint32_t QuadIndexCount = 0;

		// TODO: redesign the renderer architecture to use a single small vertex
		// buffer, sending dynamic data in as uniforms/push constants, and use INSTANCING!!

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
		struct CameraData
		{
			glm::mat4 ViewProjection;
		};

		CameraData CameraBuffer;
		Ref<UniformBufferSet> CameraUniformBuffers;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// STATS
		Renderer::Statistics Stats;
	};

	static RendererData s_Data;

	static RendererAPI* s_RendererAPI;

	void Renderer::Init()
	{
		s_RendererAPI = RendererAPI::Create();
		s_RendererAPI->Init();

		uint32_t framesInFlight = s_RendererAPI->GetFramesInFlight();
		
		#pragma region(VULKAN TUTORIAL)
		ShaderSpecification tutorialShaderSpec{};
		tutorialShaderSpec.Name = "vulkan_tutorial";
		tutorialShaderSpec.SourceDirectory = "Resources/Shaders";
		s_Data.TutorialShader = Shader::Create(tutorialShaderSpec);

		ShaderResourceManagerSpecification tutorialResourceManagerSpec{};
		tutorialResourceManagerSpec.Shader = s_Data.TutorialShader;
		tutorialResourceManagerSpec.FirstSet = 0;
		tutorialResourceManagerSpec.LastSet = 0;
		s_Data.TutorialResourceManager = ShaderResourceManager::Create(tutorialResourceManagerSpec);

		RenderPassSpecification tutorialRenderPassSpecification{};
		tutorialRenderPassSpecification.Shader = s_Data.TutorialShader;
		tutorialRenderPassSpecification.Topology = PrimitiveTopology::Triangles;
		tutorialRenderPassSpecification.HasDepthStencil = true;
		tutorialRenderPassSpecification.TargetSwapchain = false;
		tutorialRenderPassSpecification.OutputTexture = true;
		tutorialRenderPassSpecification.AttachmentWidth = s_RendererAPI->GetSwapchainWidth();
		tutorialRenderPassSpecification.AttachmentHeight = s_RendererAPI->GetSwapchainHeight();
		tutorialRenderPassSpecification.PrimaryAttachment.LoadOp = AttachmentLoadOp::Clear;
		tutorialRenderPassSpecification.PrimaryAttachment.StoreOp = AttachmentStoreOp::Store;
		tutorialRenderPassSpecification.PrimaryAttachment.Format = ImageFormat::SRGBA;
		/*tutorialRenderPassSpecification.PrimaryAttachment.InitialLayout = ImageLayout::Undefined;
		tutorialRenderPassSpecification.PrimaryAttachment.FinalLayout = ImageLayout::ColourAttachment;*/
		tutorialRenderPassSpecification.BackfaceCulling = false;
		s_Data.TutorialRenderPass = RenderPass::Create(tutorialRenderPassSpecification);

		MeshSpecification tutorialMeshSpecification{};
		tutorialMeshSpecification.Filepath = "Assets/Models/viking_room.obj";
		s_Data.TutorialMesh = StaticMesh::Create(tutorialMeshSpecification);

		uint32_t mvpSize = sizeof(MVPTransforms);

		s_Data.TutorialUniformBuffers = UniformBufferSet::Create(mvpSize, framesInFlight);

		MVPTransforms transforms{};
		for (int i = 0; i < framesInFlight; i++)
			s_Data.TutorialUniformBuffers->SetData(i, &transforms, mvpSize);

		Texture2DSpecification tutorialTextureSpec{};
		tutorialTextureSpec.ImageFilepath = "Assets/Textures/viking_room.png";
		s_Data.TutorialTexture = Texture2D::Create(tutorialTextureSpec);

		s_Data.TutorialResourceManager->ProvideResource("Matrices", s_Data.TutorialUniformBuffers);
		s_Data.TutorialResourceManager->ProvideResource("u_Texture", s_Data.TutorialTexture);
		s_Data.TutorialResourceManager->Bake();
		#pragma endregion

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TEXTURES
		s_Data.TextureSlots[0] = Texture2D::Create(1, 1);
		uint32_t flatWhite = 0xffffffff;
		s_Data.TextureSlots[0]->SetData(&flatWhite, sizeof(uint32_t));

		// TODO: setup Shader reflection and ShaderResourceManager to be capable of using arrays
		// of textures... until that's set up we'll just use slot 0 (a single pure white pixel)

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA BUFFER
		s_Data.CameraUniformBuffers = UniformBufferSet::Create(sizeof(RendererData::CameraData), framesInFlight);

		RendererData::CameraData cameraData;

		for (int i = 0; i < framesInFlight; i++)
			s_Data.CameraUniformBuffers->SetData(i, &cameraData, sizeof(RendererData::CameraData));

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// QUADS
		ShaderSpecification quadShaderSpec{};
		quadShaderSpec.Name = "quad";
		quadShaderSpec.SourceDirectory = "Resources/Shaders";
		s_Data.QuadShader = Shader::Create(quadShaderSpec, s_Data.Config.ForceShaderCompilation);
		// TODO: obtain Shaders from a Shaderlibrary instead of directly constructing them here

		ShaderResourceManagerSpecification quadResourceManagerSpec{};
		quadResourceManagerSpec.Shader = s_Data.QuadShader;
		quadResourceManagerSpec.FirstSet = 0;
		quadResourceManagerSpec.LastSet = 0;
		s_Data.QuadResourceManager = ShaderResourceManager::Create(quadResourceManagerSpec);

		s_Data.QuadResourceManager->ProvideResource("Camera", s_Data.CameraUniformBuffers);
		s_Data.QuadResourceManager->ProvideResource("u_Sampler", s_Data.TextureSlots[0]);
		s_Data.QuadResourceManager->Bake();

		RenderPassSpecification quadRenderPassSpec{};
		quadRenderPassSpec.Shader = s_Data.QuadShader;
		quadRenderPassSpec.Topology = PrimitiveTopology::Triangles;
		quadRenderPassSpec.HasDepthStencil = true;
		quadRenderPassSpec.TargetSwapchain = true;
		quadRenderPassSpec.PrimaryAttachment.LoadOp = AttachmentLoadOp::Clear;
		quadRenderPassSpec.PrimaryAttachment.StoreOp = AttachmentStoreOp::Store;
		/*quadRenderPassSpec.PrimaryAttachment.InitialLayout = ImageUsage::Undefined;
		quadRenderPassSpec.PrimaryAttachment.FinalLayout = ImageUsage::ColourAttachment;*/
		quadRenderPassSpec.BackfaceCulling = false;
		s_Data.QuadPass = RenderPass::Create(quadRenderPassSpec);

		s_Data.QuadVertexBuffers.resize(framesInFlight);
		s_Data.QuadVertexBufferBases.resize(framesInFlight);
		s_Data.QuadVertexBufferPtrs.resize(framesInFlight);

		for (uint32_t frame = 0; frame < framesInFlight; frame++)
		{
			s_Data.QuadVertexBuffers[frame] = VertexBuffer::Create(s_Data.MaxVerticesPerBuffer * sizeof(QuadVertex));
			s_Data.QuadVertexBufferPtrs[frame] = s_Data.QuadVertexBufferBases[frame] = new QuadVertex[s_Data.MaxVerticesPerBuffer];
		}

		s_Data.QuadVertexPositions[0] = { -.5f, -.5f, .0f, 1.0f };
		s_Data.QuadVertexPositions[1] = { .5f, -.5f, .0f, 1.0f };
		s_Data.QuadVertexPositions[2] = { .5f,  .5f, .0f, 1.0f };
		s_Data.QuadVertexPositions[3] = { -.5f,  .5f, .0f, 1.0f };
		
		s_Data.QuadTextureCoords[0] = { 0.0f, 0.0f };
		s_Data.QuadTextureCoords[1] = { 1.0f, 0.0f };
		s_Data.QuadTextureCoords[2] = { 1.0f, 1.0f };
		s_Data.QuadTextureCoords[3] = { 0.0f, 1.0f };

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
		// TODO: in order to implement a render queue, data like these will need a
		// dynamic lifetime, perhaps using Refs?
		delete[] quadIndices;

		// TODO: circles and lines!!

	}

	void Renderer::Shutdown()
	{
		for (uint32_t frame = 0; frame < s_RendererAPI->GetFramesInFlight(); frame++)
		{
			delete[] s_Data.QuadVertexBufferBases[frame];
			s_Data.QuadVertexBuffers[frame].Reset();
		}
		s_Data.QuadIndexBuffer.Reset();
		s_Data.QuadPass.Reset();
		s_Data.QuadResourceManager.Reset();
		s_Data.QuadShader.Reset();

		// TODO: circles and lines

		s_Data.CameraUniformBuffers.Reset();

		for (int i = 0; i < s_Data.MaxTextureSlots; i++)
			s_Data.TextureSlots[i].Reset();

		#pragma region(VULKAN TUTORIAL)
		s_Data.TutorialTexture.Reset();
		s_Data.TutorialUniformBuffers.Reset();
		s_Data.TutorialMesh.Reset();
		s_Data.TutorialRenderPass.Reset();
		s_Data.TutorialResourceManager.Reset();
		s_Data.TutorialShader.Reset();
		#pragma endregion

		s_RendererAPI->Shutdown();
	}

	RendererConfig& Renderer::GetConfig()
	{
		return s_Data.Config;
	}

	void Renderer::SetConfig(const RendererConfig& config)
	{
		s_Data.Config = config;
	}

	void Renderer::BeginFrame()
	{
		s_RendererAPI->BeginFrame();
	}

	void Renderer::EndFrame()
	{
		s_RendererAPI->EndFrame();
	}

	void Renderer::DrawTutorialScene(Ref<Texture2D>& outputTexture)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float timeSinceStart = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		float width = s_RendererAPI->GetSwapchainWidth();
		float height = s_RendererAPI->GetSwapchainHeight();

		MVPTransforms transforms{};
		transforms.Model = glm::rotate(glm::mat4(1.0f), timeSinceStart * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		transforms.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		transforms.Projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 10.0f);
		transforms.Projection[1][1] *= -1.f; // because screenspace is left-handed....

		uint32_t frameIndex = s_RendererAPI->GetCurrentFrameIndex();
		s_Data.TutorialUniformBuffers->SetData(frameIndex, &transforms, sizeof(MVPTransforms));

		s_RendererAPI->BeginRenderPass(s_Data.TutorialRenderPass);
		s_RendererAPI->TutorialDrawCalls(s_Data.TutorialRenderPass, s_Data.TutorialMesh, s_Data.TutorialResourceManager);
		s_RendererAPI->EndRenderPass(s_Data.TutorialRenderPass);

		outputTexture = s_Data.TutorialRenderPass->GetOutputTexture();

	}

	void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		/*s_Data.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));

		NewBatch();*/
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		/*s_Data.CameraBuffer.ViewProjection = camera.GetPVMatrix();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));

		NewBatch();*/
	}

	void Renderer::EndScene()
	{
		//SubmitBatch();
	}

	void Renderer::Present()
	{
		s_RendererAPI->Present();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		s_RendererAPI->OnWindowResize();
	}

	uint32_t Renderer::GetCurrentFrameIndex()
	{
		return s_RendererAPI->GetCurrentFrameIndex();
	}

	uint32_t Renderer::GetFramesInFlight()
	{
		return s_RendererAPI->GetFramesInFlight();
	}

	/*float Renderer::GetLineThickness()
	{
		return s_Data.LineThickness;
	}

	void Renderer::SetLineThickness(float thickness)
	{
		//s_Data.LineThickness = thickness;
	}*/

	void Renderer::SubmitBatch()
	{
		uint32_t frame = s_RendererAPI->GetCurrentFrameIndex();

		//if (s_Data.QuadIndexCount)
		//{
		//	uint32_t dataSize = (uint32_t)((byte*)s_Data.QuadVertexBufferPtrs[frame] - (byte*)s_Data.QuadVertexBufferBases[frame]);
		//	s_Data.QuadVertexBuffers[frame]->SetData(s_Data.QuadVertexBufferBases[frame], dataSize);

		//	// Bind textures
		//	for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
		//		s_Data.TextureSlots[i]->Bind(i);

		//	s_Data.QuadShader->Bind();
		//	//RenderCommandQueue::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
		//	s_Data.Stats.DrawCalls++;
		//}

		//if (s_Data.CircleIndexCount)
		//{
		//	uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CircleVertexBufferPtr - (uint8_t*)s_Data.CircleVertexBufferBase);
		//	s_Data.CircleVertexBuffer->SetData(s_Data.CircleVertexBufferBase, dataSize);

		//	s_Data.CircleShader->Bind();
		//	//RenderCommandQueue::DrawIndexed(s_Data.CircleVertexArray, s_Data.CircleIndexCount);
		//	s_Data.Stats.DrawCalls++;
		//}

		//if (s_Data.LineVertexCount)
		//{
		//	uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.LineVertexBufferPtr - (uint8_t*)s_Data.LineVertexBufferBase);
		//	s_Data.LineVertexBuffer->SetData(s_Data.LineVertexBufferBase, dataSize);

		//	s_Data.LineShader->Bind();
		//	RenderCommandQueue::SetLineThickness(s_Data.LineThickness);
		//	//RenderCommandQueue::DrawLines(s_Data.LineVertexArray, s_Data.LineVertexCount);
		//	s_Data.Stats.DrawCalls++;
		//}
	}

	void Renderer::NewBatch()
	{
		/*s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.CircleIndexCount = 0;
		s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;

		s_Data.LineVertexCount = 0;
		s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;

		s_Data.TextureSlotIndex = 1;*/
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// PRIMITIVES

	void Renderer::DrawQuad(const glm::mat4& transform, const glm::vec4& colour, int entityID)
	{
		//// TODO: separate maxima for each primitive type
		//if (s_Data.QuadIndexCount >= RendererData::MaxIndicesPerBuffer)
		//{
		//	SubmitBatch();
		//	NewBatch();
		//}

		//for (int i = 0; i < 4; i++)
		//{
		//	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
		//	s_Data.QuadVertexBufferPtr->Tint = colour;
		//	s_Data.QuadVertexBufferPtr->TextureCoord = s_Data.QuadTextureCoords[i];
		//	s_Data.QuadVertexBufferPtr->TextureIndex = 0.0f;
		//	s_Data.QuadVertexBufferPtr->TilingFactor = 1.0f;
		//	s_Data.QuadVertexBufferPtr->EntityID = entityID;

		//	s_Data.QuadVertexBufferPtr++;
		//}

		//s_Data.QuadIndexCount += 6;
		//s_Data.Stats.QuadCount++;
	}

	void Renderer::DrawQuad(const glm::mat4& transform, const Ref<Texture2D> texture, const glm::vec4& tint, float tiling, int entityID)
	{
		//// TODO: separate maxima for each primitive type
		//if (s_Data.QuadIndexCount >= RendererData::MaxIndicesPerBuffer)
		//{
		//	SubmitBatch();
		//	NewBatch();
		//}

		//// FIND AN AVAILABLE TEXTURE SLOT
		//float textureIndex = 0.0f;
		//{

		//	for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		//	{
		//		// TODO: this comparison is horrendous, refactor it once we have a general asset UUID system
		//		if (*s_Data.TextureSlots[i].Raw() == *texture.Raw())
		//		{
		//			textureIndex = (float)i;
		//			break;
		//		}
		//	}

		//	if (textureIndex == 0.0f)
		//	{
		//		if (s_Data.TextureSlotIndex >= RendererData::MaxTextureSlots)
		//		{
		//			SubmitBatch();
		//			NewBatch();
		//		}

		//		textureIndex = (float)s_Data.TextureSlotIndex;
		//		s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
		//		s_Data.TextureSlotIndex++;
		//	}
		//}

		//for (int i = 0; i < 4; i++)
		//{
		//	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
		//	s_Data.QuadVertexBufferPtr->Tint = tint;
		//	s_Data.QuadVertexBufferPtr->TextureCoord = s_Data.QuadTextureCoords[i];
		//	s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
		//	s_Data.QuadVertexBufferPtr->TilingFactor = tiling;
		//	s_Data.QuadVertexBufferPtr->EntityID = entityID;

		//	s_Data.QuadVertexBufferPtr++;
		//}

		//s_Data.QuadIndexCount += 6;
		//s_Data.Stats.QuadCount++;
	}

	void Renderer::DrawCircle(const glm::mat4& transform, const glm::vec4& colour, float thickness, float fade, int entityID)
	{
		//// TODO: separate maxima for each primitive type
		//if (s_Data.CircleIndexCount >= RendererData::MaxIndicesPerBuffer)
		//{
		//	SubmitBatch();
		//	NewBatch();
		//}

		//for (int i = 0; i < 4; i++)
		//{
		//	s_Data.CircleVertexBufferPtr->WorldPosition = transform * s_Data.QuadVertexPositions[i];
		//	s_Data.CircleVertexBufferPtr->LocalPosition = s_Data.QuadVertexPositions[i] * 2.0f;
		//	s_Data.CircleVertexBufferPtr->Colour = colour;
		//	s_Data.CircleVertexBufferPtr->Thickness = thickness;
		//	s_Data.CircleVertexBufferPtr->Fade = fade;
		//	s_Data.CircleVertexBufferPtr->EntityID = entityID;

		//	s_Data.CircleVertexBufferPtr++;
		//}

		//s_Data.CircleIndexCount += 6;
		//s_Data.Stats.QuadCount++;
	}

	void Renderer::DrawLine(const glm::vec3& end0, const glm::vec3& end1, const glm::vec4& colour, int entityID)
	{
		//// TODO: separate maxima for each primitive type
		//if (s_Data.LineVertexCount >= RendererData::MaxVerticesPerBuffer)
		//{
		//	SubmitBatch();
		//	NewBatch();
		//}

		//s_Data.LineVertexBufferPtr->Position = end0;
		//s_Data.LineVertexBufferPtr->Colour = colour;
		//s_Data.LineVertexBufferPtr->EntityID = entityID;
		//s_Data.LineVertexBufferPtr++;

		//s_Data.LineVertexBufferPtr->Position = end1;
		//s_Data.LineVertexBufferPtr->Colour = colour;
		//s_Data.LineVertexBufferPtr->EntityID = entityID;
		//s_Data.LineVertexBufferPtr++;

		//s_Data.LineVertexCount += 2;
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
		return s_Data.Stats;
	}

	void Renderer::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Renderer::Statistics));
	}
	

}
