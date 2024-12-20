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
	struct TestTransforms
	{
		glm::mat4 Model;
		glm::mat4 View;
		glm::mat4 Projection;
	};

	struct RendererData
	{
		#pragma region(VULKAN TESTS)
		Ref<Shader> TestShader;
		Ref<ShaderResourceManager> TestResourceManager;
		Ref<RenderPass> TestRenderPass;
		Ref<StaticMesh> TestMesh;
		Ref<UniformBufferSet> TestUniformBuffers;
		Ref<Texture2D> TestTexture;
		#pragma endregion

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CONFIG
		RendererConfig Config;

		// TODO: migrate these into RenderConfig
		static const uint32_t MaxQuadsPerBatch = 10000;
		static const uint32_t MaxVerticesPerBatch = MaxQuadsPerBatch * 4;
		static const uint32_t MaxIndicesPerBatch = MaxQuadsPerBatch * 6;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TEXTURES
		std::vector<Ref<Texture2D>> TextureSlots;
		// TODO: do I need this index?
		uint32_t CurrentTextureSlotIndex = 1; // start at 1, because slot 0 will be our default 1x1 white texture

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA
		struct CameraData
		{
			glm::mat4 View;
			glm::mat4 Projection;
		};

		// TODO: uniform buffer vs push constants for camera
		CameraData CameraBuffer;
		Ref<UniformBufferSet> CameraUniformBuffers;
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// QUADS
		Ref<Shader> QuadShader;
		Ref<ShaderResourceManager> QuadResourceManager;
		Ref<RenderPass> QuadRenderPass;

		Ref<IndexBuffer> QuadIndexBuffer;
		uint32_t QuadIndexCount;

		std::vector<std::vector<Ref<VertexBuffer>>>	QuadVertexBuffers;
		QuadVertex* QuadBatchStart;
		QuadVertex* QuadBatchEnd;
		glm::vec4 QuadPositions[4];
		glm::vec2 QuadTextureCoords[4];

		#pragma region(RESSURECT CIRCLES/LINES)
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
		#pragma endregion

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
		
		#pragma region(VULKAN TESTS)
		ShaderSpecification testShaderSpec{};
		testShaderSpec.Name = "vulkan_tutorial";
		testShaderSpec.SourceDirectory = "Resources/Shaders";
		s_Data.TestShader = Shader::Create(testShaderSpec);

		ShaderResourceManagerSpecification testResourceManagerSpec{};
		testResourceManagerSpec.Shader = s_Data.TestShader;
		testResourceManagerSpec.FirstSet = 0;
		testResourceManagerSpec.LastSet = 0;
		s_Data.TestResourceManager = ShaderResourceManager::Create(testResourceManagerSpec);

		/*RenderPassSpecification testRenderPassSpecification{};
		testRenderPassSpecification.Shader = s_Data.TestShader;
		testRenderPassSpecification.Topology = PrimitiveTopology::Triangles;
		testRenderPassSpecification.FramebufferSpec.HasDepthStencil = true;
		testRenderPassSpecification.FramebufferSpec.TargetSwapchain = true;
		testRenderPassSpecification.FramebufferSpec.Width = s_RendererAPI->GetSwapchainWidth();
		testRenderPassSpecification.FramebufferSpec.Height = s_RendererAPI->GetSwapchainHeight();
		{
			auto& colourAttachmentSpec = testRenderPassSpecification.FramebufferSpec.ColourAttachmentSpecs.emplace_back();
			colourAttachmentSpec.Format = ImageFormat::SRGBA;
			colourAttachmentSpec.ClearColour = { 0.0f, 0.0f, 0.0f };
			colourAttachmentSpec.LoadOp = AttachmentLoadOp::Clear;
			colourAttachmentSpec.StoreOp = AttachmentStoreOp::Store;
			colourAttachmentSpec.InitialLayout = ImageLayout::Unspecified;
			colourAttachmentSpec.FinalLayout = ImageLayout::ColourAttachment;
		}
		{
			testRenderPassSpecification.FramebufferSpec.DepthStencilAttachmentSpec.LoadOp = AttachmentLoadOp::Clear;
			testRenderPassSpecification.FramebufferSpec.DepthStencilAttachmentSpec.StoreOp = AttachmentStoreOp::Store;
			testRenderPassSpecification.FramebufferSpec.DepthStencilAttachmentSpec.InitialLayout = ImageLayout::Unspecified;
			testRenderPassSpecification.FramebufferSpec.DepthStencilAttachmentSpec.FinalLayout = ImageLayout::DepthStencilAttachment;
		}
		testRenderPassSpecification.BackfaceCulling = false;
		s_Data.TestRenderPass = RenderPass::Create(testRenderPassSpecification);*/

		MeshSpecification tutorialMeshSpecification{};
		tutorialMeshSpecification.Filepath = "Assets/Models/viking_room.obj";
		s_Data.TestMesh = StaticMesh::Create(tutorialMeshSpecification);

		uint32_t mvpSize = sizeof(TestTransforms);

		s_Data.TestUniformBuffers = UniformBufferSet::Create(mvpSize, framesInFlight);

		TestTransforms transforms{};
		for (int frame = 0; frame < framesInFlight; frame++)
			s_Data.TestUniformBuffers->SetData(frame, &transforms, mvpSize);

		Texture2DSpecification tutorialTextureSpec{};
		s_Data.TestTexture = Texture2D::CreateFromFile(tutorialTextureSpec, "Assets/Textures/viking_room.png");

		s_Data.TestResourceManager->ProvideResource("Matrices", s_Data.TestUniformBuffers);
		s_Data.TestResourceManager->ProvideResource("u_Texture", s_Data.TestTexture);
		s_Data.TestResourceManager->Bake();
		#pragma endregion

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TEXTURES
		s_Data.TextureSlots.resize(s_Data.Config.MaximumBoundTextures);

		Texture2DSpecification flatWhite{};
		s_Data.TextureSlots[0] = Texture2D::CreateFlatColourTexture(flatWhite, 1);
		
		//s_Data.TextureSlots[1] = s_Data.TestRenderPass->GetOutputTexture();

		// TODO: setup Shader reflection and ShaderResourceManager to be capable of using arrays
		// of textures... until that's set up we'll just use slot 0 (a single pure white pixel)

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA BUFFER
		s_Data.CameraUniformBuffers = UniformBufferSet::Create(sizeof(RendererData::CameraData), framesInFlight);

		RendererData::CameraData cameraData{};
		for (int frame = 0; frame < framesInFlight; frame++)
			s_Data.CameraUniformBuffers->SetData(frame, &cameraData, sizeof(RendererData::CameraData));

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
 
		/*RenderPassSpecification quadRenderPassSpec{};
		quadRenderPassSpec.Shader = s_Data.QuadShader;
		quadRenderPassSpec.Topology = PrimitiveTopology::Triangles;
		quadRenderPassSpec.FramebufferSpec.HasDepthStencil = true;
		quadRenderPassSpec.FramebufferSpec.TargetSwapchain = true;
		quadRenderPassSpec.FramebufferSpec.Width = s_RendererAPI->GetSwapchainWidth();
		quadRenderPassSpec.FramebufferSpec.Height = s_RendererAPI->GetSwapchainHeight();
		{
			auto& colourAttachmentSpec = quadRenderPassSpec.FramebufferSpec.ColourAttachmentSpecs.emplace_back();

			colourAttachmentSpec.Format = ImageFormat::SRGBA;
			colourAttachmentSpec.LoadOp = AttachmentLoadOp::Load;
			colourAttachmentSpec.StoreOp = AttachmentStoreOp::Store;
			colourAttachmentSpec.InitialLayout = ImageLayout::ColourAttachment;
			colourAttachmentSpec.FinalLayout = ImageLayout::Presentation;
		}
		{
			quadRenderPassSpec.FramebufferSpec.DepthStencilAttachmentSpec.InheritFrom = s_Data.TestRenderPass->GetFramebuffer()->GetDepthStencilAttachment();
			quadRenderPassSpec.FramebufferSpec.DepthStencilAttachmentSpec.LoadOp = AttachmentLoadOp::Load;
			quadRenderPassSpec.FramebufferSpec.DepthStencilAttachmentSpec.StoreOp = AttachmentStoreOp::Unspecified;
			quadRenderPassSpec.FramebufferSpec.DepthStencilAttachmentSpec.InitialLayout = ImageLayout::DepthStencilAttachment;
			quadRenderPassSpec.FramebufferSpec.DepthStencilAttachmentSpec.FinalLayout = ImageLayout::DepthStencilAttachment;
		}
		quadRenderPassSpec.BackfaceCulling = false;
		s_Data.QuadRenderPass = RenderPass::Create(quadRenderPassSpec);*/

		uint32_t* quadIndices = new uint32_t[s_Data.MaxIndicesPerBatch];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndicesPerBatch; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}
		s_Data.QuadIndexBuffer = IndexBuffer::Create(quadIndices, s_Data.MaxIndicesPerBatch);
		delete[] quadIndices; // TODO: if/when I implement a separate render thread, this may need to be dynamically allocated

		s_Data.QuadVertexBuffers.resize(framesInFlight);

		s_Data.QuadBatchEnd = s_Data.QuadBatchStart = new QuadVertex[s_Data.MaxVerticesPerBatch];

		s_Data.QuadPositions[0] = { -.5f, -.5f, .0f, 1.0f };
		s_Data.QuadPositions[1] = { .5f, -.5f, .0f, 1.0f };
		s_Data.QuadPositions[2] = { .5f,  .5f, .0f, 1.0f };
		s_Data.QuadPositions[3] = { -.5f,  .5f, .0f, 1.0f };
		
		s_Data.QuadTextureCoords[0] = { 0.0f, 0.0f };
		s_Data.QuadTextureCoords[1] = { 1.0f, 0.0f };
		s_Data.QuadTextureCoords[2] = { 1.0f, 1.0f };
		s_Data.QuadTextureCoords[3] = { 0.0f, 1.0f };

		//// TODO: circles and lines!!

	}

	void Renderer::Shutdown()
	{
		delete[] s_Data.QuadBatchStart;
		s_Data.QuadBatchEnd = s_Data.QuadBatchStart = nullptr;

		for (uint32_t frame = 0; frame < s_RendererAPI->GetFramesInFlight(); frame++)
		{
			for (auto& vb : s_Data.QuadVertexBuffers[frame])
				vb.Reset();
		}
		s_Data.QuadVertexBuffers.clear();

		s_Data.QuadIndexBuffer.Reset();
		s_Data.QuadRenderPass.Reset();
		s_Data.QuadResourceManager.Reset();
		s_Data.QuadShader.Reset();

		// TODO: circles and lines

		s_Data.CameraUniformBuffers.Reset();

		for (auto& texture : s_Data.TextureSlots)
			texture.Reset();
		s_Data.TextureSlots.clear();

		#pragma region(VULKAN TESTS)
		s_Data.TestTexture.Reset();
		s_Data.TestUniformBuffers.Reset();
		s_Data.TestMesh.Reset();
		s_Data.TestRenderPass.Reset();
		s_Data.TestResourceManager.Reset();
		s_Data.TestShader.Reset();
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
		uint32_t frame = s_RendererAPI->GetCurrentFrameIndex();

		s_Data.QuadVertexBuffers[frame].clear();
		s_Data.QuadIndexCount = 0;

		BeginNewQuadBatch();
		/*BeginNewCircleBatch();
		BeginNewLineBatch();*/
	}

	void Renderer::EndFrame()
	{
		SubmitCurrentQuadBatch();
		/*SubmitCurrentCircleBatch();
		SubmitCurrentLineBatch();*/

		FlushDrawCalls();

		s_RendererAPI->EndFrame();
	}

	void Renderer::BeginNewQuadBatch()
	{
		s_Data.QuadIndexCount = 0;
		s_Data.QuadBatchEnd = s_Data.QuadBatchStart;

		/*s_Data.CircleIndexCount = 0;
		s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;

		s_Data.LineVertexCount = 0;
		s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;

		s_Data.CurrentTextureSlotIndex = 1;*/
	}

	void Renderer::SubmitCurrentQuadBatch()
	{
		if (s_Data.QuadIndexCount == 0)
			return;

		uint32_t frame = s_RendererAPI->GetCurrentFrameIndex(); 
		uint32_t dataSize = (uint64_t)((byte*)s_Data.QuadBatchEnd - (byte*)s_Data.QuadBatchStart);

		s_Data.QuadVertexBuffers[frame].emplace_back(VertexBuffer::Create(s_Data.QuadBatchStart, dataSize));

		s_Data.Stats.QuadBatchCount++;
	}

	void Renderer::FlushDrawCalls()
	{
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float timeSinceStart = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		float width = s_RendererAPI->GetSwapchainWidth();
		float height = s_RendererAPI->GetSwapchainHeight();
		uint32_t frame = s_RendererAPI->GetCurrentFrameIndex();

		s_Data.CameraBuffer.View = glm::lookAt(glm::vec3(0.0f, 0.001f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		s_Data.CameraBuffer.Projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
		s_Data.CameraBuffer.Projection[1][1] *= -1.f; // because screenspace is left-handed....

		s_Data.CameraUniformBuffers->SetData(frame, &s_Data.CameraBuffer, sizeof(RendererData::CameraData));

		s_RendererAPI->BeginRenderPass(s_Data.QuadRenderPass);
		for (auto& batch : s_Data.QuadVertexBuffers[frame])
		{
			s_RendererAPI->DrawIndexed(s_Data.QuadRenderPass, s_Data.QuadResourceManager, batch, s_Data.QuadIndexBuffer, s_Data.QuadIndexCount);
		}
		s_RendererAPI->EndRenderPass();

		// TODO: circle/line draw calls
	}

	void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		/*s_Data.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));*/
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		/*s_Data.CameraBuffer.ViewProjection = camera.GetPVMatrix();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));*/
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Present()
	{
		s_RendererAPI->Present();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		s_RendererAPI->OnWindowResize();
	}

	uint32_t Renderer::GetSwapchainWidth()
	{
		return s_RendererAPI->GetSwapchainWidth();
	}

	uint32_t Renderer::GetSwapchainHeight()
	{
		return s_RendererAPI->GetSwapchainHeight();
	}

	uint32_t Renderer::GetCurrentFrameIndex()
	{
		return s_RendererAPI->GetCurrentFrameIndex();
	}

	uint32_t Renderer::GetFramesInFlight()
	{
		return s_RendererAPI->GetFramesInFlight();
	}

	void Renderer::DrawToSwapchain(Ref<Image2D> frameOutput)
	{

	}

	void Renderer::DrawTutorialScene()
	{
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float timeSinceStart = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		float width = s_RendererAPI->GetSwapchainWidth();
		float height = s_RendererAPI->GetSwapchainHeight();
		uint32_t frameIndex = s_RendererAPI->GetCurrentFrameIndex();

		TestTransforms transforms{};
		transforms.Model = glm::rotate(glm::mat4(1.0f), timeSinceStart * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		transforms.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		transforms.Projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
		transforms.Projection[1][1] *= -1.f; // because screenspace is left-handed....

		s_Data.TestUniformBuffers->SetData(frameIndex, &transforms, sizeof(TestTransforms));

		s_RendererAPI->BeginRenderPass(s_Data.TestRenderPass);
		s_RendererAPI->DrawMesh(s_Data.TestRenderPass, s_Data.TestResourceManager, s_Data.TestMesh);
		s_RendererAPI->EndRenderPass();
	}

	/*float Renderer::GetLineThickness()
	{
		return s_Data.LineThickness;
	}

	void Renderer::SetLineThickness(float thickness)
	{
		//s_Data.LineThickness = thickness;
	}*/

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// PRIMITIVES

	void Renderer::DrawQuad(const glm::mat4& transform, const glm::vec4& colour, int entityID)
	{
		if (s_Data.QuadIndexCount >= RendererData::MaxIndicesPerBatch)
		{
			SubmitCurrentQuadBatch();
			BeginNewQuadBatch();
		}

		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadBatchEnd->Position = transform * s_Data.QuadPositions[i];
			s_Data.QuadBatchEnd->Tint = colour;
			s_Data.QuadBatchEnd->TextureCoord = s_Data.QuadTextureCoords[i];
			/*s_Data.QuadBatchEnd->TextureIndex = 0.0f;
			s_Data.QuadBatchEnd->TilingFactor = 1.0f;
			s_Data.QuadBatchEnd->EntityID = entityID;*/

			s_Data.QuadBatchEnd++;
		}

		s_Data.QuadIndexCount += 6;
		s_Data.Stats.QuadCount++;
	}

	void Renderer::DrawQuad(const glm::mat4& transform, const Ref<Texture2D> texture, const glm::vec4& tint, float tiling, int entityID)
	{
		//// TODO: separate maxima for each primitive type
		//if (s_Data.QuadIndexCount >= RendererData::MaxIndicesPerBatch)
		//{
		//	SubmitCurrentQuadBatch();
		//	BeginNewQuadBatch();
		//}

		//// FIND AN AVAILABLE TEXTURE SLOT
		//float textureIndex = 0.0f;
		//{

		//	for (uint32_t i = 1; i < s_Data.CurrentTextureSlotIndex; i++)
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
		//		if (s_Data.CurrentTextureSlotIndex >= RendererData::MaximumBoundTextures)
		//		{
		//			SubmitCurrentQuadBatch();
		//			BeginNewQuadBatch();
		//		}

		//		textureIndex = (float)s_Data.CurrentTextureSlotIndex;
		//		s_Data.TextureSlots[s_Data.CurrentTextureSlotIndex] = texture;
		//		s_Data.CurrentTextureSlotIndex++;
		//	}
		//}

		//for (int i = 0; i < 4; i++)
		//{
		//	s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadPositions[i];
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
		//if (s_Data.CircleIndexCount >= RendererData::MaxIndicesPerBatch)
		//{
		//	SubmitCurrentQuadBatch();
		//	BeginNewQuadBatch();
		//}

		//for (int i = 0; i < 4; i++)
		//{
		//	s_Data.CircleVertexBufferPtr->WorldPosition = transform * s_Data.QuadPositions[i];
		//	s_Data.CircleVertexBufferPtr->LocalPosition = s_Data.QuadPositions[i] * 2.0f;
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
		//if (s_Data.LineVertexCount >= RendererData::MaxVerticesPerBatch)
		//{
		//	SubmitCurrentQuadBatch();
		//	BeginNewQuadBatch();
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
