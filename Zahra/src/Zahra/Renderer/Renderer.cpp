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
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// GENERAL
		RendererConfig Config;

		const uint32_t MaxBatchSize = 10000; // TODO: move to Config
		const uint32_t MaxQuadVerticesPerBatch = MaxBatchSize * 4;
		const uint32_t MaxQuadIndicesPerBatch = MaxBatchSize * 6;

		Renderer::Statistics Stats;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// FRAMEBUFFERS
		Ref<Image2D> RenderTarget;
		Ref<Framebuffer> ClearPassFramebuffer, LoadPassFramebuffer;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA
		struct CameraData
		{
			glm::mat4 View = glm::mat4(1.0f);
			glm::mat4 Projection = glm::mat4(1.0f);
		};

		// TODO: uniform buffer vs push constants for camera
		CameraData CameraBuffer{};
		Ref<UniformBufferSet> CameraUniformBuffers;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TEST
		Ref<Shader> TestShader;
		Ref<ShaderResourceManager> TestResourceManager;
		Ref<RenderPass> TestRenderPass;
		Ref<StaticMesh> TestMesh;
		Ref<Texture2D> TestTexture;
		Ref<UniformBufferSet> TestUniformBuffers;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CLEAR PASS (lazy placeholder until I have render passes prior to quads)
		Ref<VertexBuffer> ClearVertex;
		Ref<IndexBuffer> ClearIndices;
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// QUADS
		std::vector<Ref<Texture2D>> TextureSlots;
		// TODO: do I need this index?
		uint32_t CurrentTextureSlotIndex = 1; // start at 1, because slot 0 will be our default 1x1 white texture

		Ref<Shader> QuadShader;
		Ref<ShaderResourceManager> QuadResourceManager;
		Ref<RenderPass> QuadRenderPass;

		std::vector<std::vector<Ref<VertexBuffer>>>	QuadVertexBuffers; // indexed by (batch, frame)
		std::vector<QuadVertex*> QuadBatchStarts;
		std::vector<QuadVertex*> QuadBatchEnds;
		uint32_t BatchIndex = 0;

		Ref<IndexBuffer> QuadIndexBuffer;
		uint32_t QuadIndexCount = 0;

		glm::vec4 QuadPositions[4]{};
		glm::vec2 QuadTextureCoords[4]{};

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
		// SWAPCHAIN RENDERING
		Ref<Shader> SwapchainShader;
		Ref<ShaderResourceManager> SwapchainResourceManager;
		Ref<RenderPass> SwapchainRenderPass;
		Ref<Texture2D> SwapchainTexture;
		
	};

	static RendererData s_Data;

	static RendererAPI* s_RendererAPI;

	void Renderer::Init()
	{
		s_RendererAPI = RendererAPI::Create();
		s_RendererAPI->Init();

		uint32_t framesInFlight = s_RendererAPI->GetFramesInFlight();

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// FRAMEBUFFERS
		{
			Image2DSpecification defaultRenderTargetSpec{};
			defaultRenderTargetSpec.Name = "default_render_target";
			defaultRenderTargetSpec.Format = ImageFormat::SRGBA;
			defaultRenderTargetSpec.Width = s_RendererAPI->GetSwapchainWidth();
			defaultRenderTargetSpec.Height = s_RendererAPI->GetSwapchainHeight();
			defaultRenderTargetSpec.Sampled = true;
			s_Data.RenderTarget = Image2D::Create(defaultRenderTargetSpec);

			FramebufferSpecification framebufferSpec{};
			framebufferSpec.Name = "clear_framebuffer";
			framebufferSpec.Width = s_RendererAPI->GetSwapchainWidth();
			framebufferSpec.Height = s_RendererAPI->GetSwapchainHeight();
			framebufferSpec.ClearColour = { .0f, .0f, .0f };
			{
				auto& attachment = framebufferSpec.ColourAttachmentSpecs.emplace_back();
				attachment.InheritFrom = s_Data.RenderTarget;
				attachment.Format = ImageFormat::SRGBA;
				attachment.LoadOp = AttachmentLoadOp::Clear;
				attachment.StoreOp = AttachmentStoreOp::Store;
			}
			framebufferSpec.HasDepthStencil = true;
			framebufferSpec.DepthClearValue = 1.0f;
			{
				framebufferSpec.DepthStencilAttachmentSpec.Format = ImageFormat::DepthStencil;
				framebufferSpec.DepthStencilAttachmentSpec.LoadOp = AttachmentLoadOp::Clear;
				framebufferSpec.DepthStencilAttachmentSpec.StoreOp = AttachmentStoreOp::Store;
			}
			s_Data.ClearPassFramebuffer = Framebuffer::Create(framebufferSpec);

			framebufferSpec.Name = "load_framebuffer";
			{
				framebufferSpec.ColourAttachmentSpecs[0].LoadOp = AttachmentLoadOp::Load;
			}
			{
				framebufferSpec.DepthStencilAttachmentSpec.InheritFrom = s_Data.ClearPassFramebuffer->GetDepthStencilAttachment();
				framebufferSpec.DepthStencilAttachmentSpec.LoadOp = AttachmentLoadOp::Load;
			}
			s_Data.LoadPassFramebuffer = Framebuffer::Create(framebufferSpec);
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA
		{
			s_Data.CameraUniformBuffers = UniformBufferSet::Create(sizeof(RendererData::CameraData), framesInFlight);

			RendererData::CameraData cameraData{};
			for (uint32_t frame = 0; frame < framesInFlight; frame++)
				s_Data.CameraUniformBuffers->SetData(frame, &cameraData, sizeof(RendererData::CameraData));
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TEST
		{
			ShaderSpecification testShaderSpec{};
			testShaderSpec.Name = "vulkan_tutorial";
			s_Data.TestShader = Shader::Create(testShaderSpec);

			ShaderResourceManagerSpecification testResourceManagerSpec{};
			testResourceManagerSpec.Shader = s_Data.TestShader;
			testResourceManagerSpec.FirstSet = 0;
			testResourceManagerSpec.LastSet = 0;
			s_Data.TestResourceManager = ShaderResourceManager::Create(testResourceManagerSpec);

			RenderPassSpecification testRenderPassSpecification{};
			testRenderPassSpecification.Name = "test_pass";
			testRenderPassSpecification.Shader = s_Data.TestShader;
			testRenderPassSpecification.RenderTarget = s_Data.ClearPassFramebuffer;
			testRenderPassSpecification.Topology = PrimitiveTopology::Triangles;
			testRenderPassSpecification.BackfaceCulling = true;
			s_Data.TestRenderPass = RenderPass::Create(testRenderPassSpecification);

			MeshSpecification tutorialMeshSpecification{};
			tutorialMeshSpecification.Name = "viking_room.obj";
			tutorialMeshSpecification.Filepath = "Assets/Models/viking_room.obj";
			s_Data.TestMesh = StaticMesh::Create(tutorialMeshSpecification);

			uint32_t mvpSize = sizeof(TestTransforms);

			s_Data.TestUniformBuffers = UniformBufferSet::Create(mvpSize, framesInFlight);

			TestTransforms transforms{};
			for (uint32_t frame = 0; frame < framesInFlight; frame++)
				s_Data.TestUniformBuffers->SetData(frame, &transforms, mvpSize);

			Texture2DSpecification tutorialTextureSpec{};
			s_Data.TestTexture = Texture2D::CreateFromFile(tutorialTextureSpec, "Assets/Textures/viking_room.png");

			s_Data.TestResourceManager->ProvideResource("Matrices", s_Data.TestUniformBuffers);
			s_Data.TestResourceManager->ProvideResource("u_Texture", s_Data.TestTexture);
			s_Data.TestResourceManager->Bake();
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CLEAR PASS
		{
			/*Ref<VertexBuffer> ClearVertex;
			Ref<IndexBuffer> ClearIndices;*/

			MeshVertex vertex = { {.0f, .0f, 1000.0f}, {.0f, .0f, .0f}, {.0f, .0f} };
			s_Data.ClearVertex = VertexBuffer::Create(&vertex, sizeof(MeshVertex));
			uint32_t indices[3] = { 0,0,0 };
			s_Data.ClearIndices = IndexBuffer::Create(indices, 3);
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// QUADS
		{
			s_Data.TextureSlots.resize(s_Data.Config.MaximumBoundTextures);

			Texture2DSpecification flatWhite{};
			s_Data.TextureSlots[0] = Texture2D::CreateFlatColourTexture(flatWhite, 0xffffffff);

			// TODO: setup Shader reflection and ShaderResourceManager to be capable of using arrays
			// of textures... until that's set up we'll just use slot 0 (a single pure white pixel)

			ShaderSpecification quadShaderSpec{};
			quadShaderSpec.Name = "quad";
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
			quadRenderPassSpec.Name = "quad_pass";
			quadRenderPassSpec.Shader = s_Data.QuadShader;
			quadRenderPassSpec.RenderTarget = s_Data.LoadPassFramebuffer;
			quadRenderPassSpec.Topology = PrimitiveTopology::Triangles;
			quadRenderPassSpec.BackfaceCulling = false;
			s_Data.QuadRenderPass = RenderPass::Create(quadRenderPassSpec);

			s_Data.QuadVertexBuffers.resize(1);
			s_Data.QuadVertexBuffers[0].resize(framesInFlight);
			for (uint32_t frame = 0; frame < framesInFlight; frame++)
			{
				s_Data.QuadVertexBuffers[0][frame] = VertexBuffer::Create(s_Data.MaxQuadVerticesPerBatch * sizeof(QuadVertex));
			}

			s_Data.QuadBatchStarts.resize(1);
			s_Data.QuadBatchEnds.resize(1);
			s_Data.QuadBatchStarts[0] = znew QuadVertex[s_Data.MaxQuadVerticesPerBatch];

			uint32_t* quadIndices = znew uint32_t[s_Data.MaxQuadIndicesPerBatch];
			uint32_t offset = 0;
			for (uint32_t i = 0; i < s_Data.MaxQuadIndicesPerBatch; i += 6)
			{
				quadIndices[i + 0] = offset + 0;
				quadIndices[i + 1] = offset + 1;
				quadIndices[i + 2] = offset + 2;

				quadIndices[i + 3] = offset + 2;
				quadIndices[i + 4] = offset + 3;
				quadIndices[i + 5] = offset + 0;

				offset += 4;
			}
			s_Data.QuadIndexBuffer = IndexBuffer::Create(quadIndices, s_Data.MaxQuadIndicesPerBatch);
			delete[] quadIndices;

			s_Data.QuadIndexCount = 0;

			s_Data.QuadPositions[0] = { -.5f, -.5f, .0f, 1.0f };
			s_Data.QuadPositions[1] = {  .5f, -.5f, .0f, 1.0f };
			s_Data.QuadPositions[2] = {  .5f,  .5f, .0f, 1.0f };
			s_Data.QuadPositions[3] = { -.5f,  .5f, .0f, 1.0f };

			s_Data.QuadTextureCoords[0] = { 0.0f, 0.0f };
			s_Data.QuadTextureCoords[1] = { 1.0f, 0.0f };
			s_Data.QuadTextureCoords[2] = { 1.0f, 1.0f };
			s_Data.QuadTextureCoords[3] = { 0.0f, 1.0f };
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLES
		{
			// TODO:
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// LINES
		{
			// TODO:
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// SWAPCHAIN
		{
			ShaderSpecification swapchainShaderSpec{};
			swapchainShaderSpec.Name = "fullscreen_triangle";
			s_Data.SwapchainShader = Shader::Create(swapchainShaderSpec);

			ShaderResourceManagerSpecification swapchainResourceManagerSpec{};
			swapchainResourceManagerSpec.Shader = s_Data.SwapchainShader;
			swapchainResourceManagerSpec.FirstSet = 0;
			swapchainResourceManagerSpec.LastSet = 0;
			s_Data.SwapchainResourceManager = ShaderResourceManager::Create(swapchainResourceManagerSpec);

			RenderPassSpecification swapchainRenderPassSpec{};
			swapchainRenderPassSpec.Name = "swapchain_pass";
			swapchainRenderPassSpec.Shader = s_Data.SwapchainShader;
			swapchainRenderPassSpec.Topology = PrimitiveTopology::Triangles;
			swapchainRenderPassSpec.BackfaceCulling = false;
			s_Data.SwapchainRenderPass = RenderPass::Create(swapchainRenderPassSpec);

			// TODO: set this image externally?
			s_Data.SwapchainTexture = Texture2D::CreateFromImage2D(s_Data.LoadPassFramebuffer->GetColourAttachment(0));

			s_Data.SwapchainResourceManager->ProvideResource("u_Sampler", s_Data.SwapchainTexture);
			s_Data.SwapchainResourceManager->Bake();

		}

	}

	void Renderer::Shutdown()
	{
		uint32_t framesInFlight = GetFramesInFlight();

		s_Data.SwapchainTexture.Reset();
		s_Data.SwapchainRenderPass.Reset();
		s_Data.SwapchainResourceManager.Reset();
		s_Data.SwapchainShader.Reset();

		// TODO: cleanup circle and line data

		s_Data.QuadIndexBuffer.Reset();

		s_Data.QuadBatchEnds.clear();
		for (auto batch : s_Data.QuadBatchStarts)
		{
			zdelete[] batch;
		}
		s_Data.QuadBatchStarts.clear();

		for (auto batch : s_Data.QuadVertexBuffers)
		{
			for (uint32_t frame = 0; frame < framesInFlight; frame++)
			{
				batch[frame].Reset();
			}
			batch.clear();
		}
		s_Data.QuadVertexBuffers.clear();
		
		s_Data.QuadRenderPass.Reset();
		s_Data.QuadResourceManager.Reset();
		s_Data.QuadShader.Reset();

		for (auto texture : s_Data.TextureSlots)
		{
			texture.Reset();
		}
		s_Data.TextureSlots.clear();

		s_Data.ClearIndices.Reset();
		s_Data.ClearVertex.Reset();

		s_Data.TestUniformBuffers.Reset();
		s_Data.TestTexture.Reset();
		s_Data.TestMesh.Reset();
		s_Data.TestRenderPass.Reset();
		s_Data.TestResourceManager.Reset();
		s_Data.TestShader.Reset();

		s_Data.CameraUniformBuffers.Reset();

		s_Data.LoadPassFramebuffer.Reset();
		s_Data.ClearPassFramebuffer.Reset();
		s_Data.RenderTarget.Reset();

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
		s_RendererAPI->BeginRenderPass(s_Data.SwapchainRenderPass);
		s_RendererAPI->FinalDrawCall(s_Data.SwapchainRenderPass, s_Data.SwapchainResourceManager);
		s_RendererAPI->EndRenderPass();

		s_RendererAPI->EndFrame();
	}

	void Renderer::BeginScene(const glm::mat4& view, const glm::mat4& projection)
	{
		uint32_t frame = s_RendererAPI->GetCurrentFrameIndex();

		s_Data.CameraBuffer.View = view;
		s_Data.CameraBuffer.Projection = projection;
		s_Data.CameraUniformBuffers->SetData(frame, &s_Data.CameraBuffer, sizeof(RendererData::CameraData));

		s_Data.QuadIndexCount = 0;

		for (uint32_t batch = 0; batch < s_Data.QuadBatchEnds.size(); batch++)
		{
			s_Data.QuadBatchEnds[batch] = s_Data.QuadBatchStarts[batch];
		}

		// TODO: reset texture array
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
		uint32_t frame = s_RendererAPI->GetCurrentFrameIndex();

		s_RendererAPI->BeginRenderPass(s_Data.QuadRenderPass);

		for (uint32_t batch = 0; batch <= s_Data.BatchIndex; batch++)
		{
			uint32_t dataSize = (uint32_t)((byte*)s_Data.QuadBatchEnds[batch] - (byte*)s_Data.QuadBatchStarts[batch]);
			if (dataSize)
			{
				uint32_t batchSize = (batch == s_Data.BatchIndex) ?
					s_Data.QuadIndexCount - batch * s_Data.MaxQuadIndicesPerBatch :
					s_Data.MaxQuadIndicesPerBatch;

				s_Data.QuadVertexBuffers[batch][frame]->SetData(s_Data.QuadBatchStarts[batch], dataSize);

				// TODO: assign ALL textures in array
				s_Data.QuadResourceManager->ProvideResource("u_Sampler", s_Data.TextureSlots[0]);

				// TODO: compare this to moving the begin/end render pass outside of the for loop
				
				s_RendererAPI->DrawIndexed(s_Data.QuadRenderPass, s_Data.QuadResourceManager, s_Data.QuadVertexBuffers[batch][frame], s_Data.QuadIndexBuffer, batchSize);
				
				s_Data.Stats.DrawCalls++;
			}
		}

		s_RendererAPI->EndRenderPass();
	}


	void Renderer::Present()
	{
		s_RendererAPI->Present();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		s_RendererAPI->OnWindowResize();

		s_Data.RenderTarget->Resize(width, height);
		s_Data.ClearPassFramebuffer->Resize(width, height);
		s_Data.LoadPassFramebuffer->Resize(width, height);
		s_Data.TestRenderPass->OnResize();
		s_Data.QuadRenderPass->OnResize();

		if (s_Data.SwapchainTexture)
		{
			s_Data.SwapchainTexture->Resize(width, height);
			s_Data.SwapchainResourceManager->ProvideResource("u_Sampler", s_Data.SwapchainTexture);
			s_Data.SwapchainResourceManager->Bake();
		}
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

	const Ref<Image2D>& Renderer::GetRenderTarget()
	{
		return s_Data.RenderTarget;
	}

	void Renderer::DrawTestScene()
	{
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float timeSinceStart = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		float width = (float)s_RendererAPI->GetSwapchainWidth();
		float height = (float)s_RendererAPI->GetSwapchainHeight();
		uint32_t frameIndex = s_RendererAPI->GetCurrentFrameIndex();

		TestTransforms transforms{};
		transforms.Model = glm::rotate(glm::mat4(1.0f), timeSinceStart * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		transforms.View = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		transforms.Projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
		transforms.Projection[1][1] *= -1.f; // because screenspace is left-handed....

		s_Data.TestUniformBuffers->SetData(frameIndex, &transforms, sizeof(TestTransforms));

		s_RendererAPI->BeginRenderPass(s_Data.TestRenderPass);
		s_RendererAPI->DrawMesh(s_Data.TestRenderPass, s_Data.TestResourceManager, s_Data.TestMesh);
		s_RendererAPI->EndRenderPass();
	}

	void Renderer::ClearPass()
	{
		uint32_t frameIndex = s_RendererAPI->GetCurrentFrameIndex();

		TestTransforms transforms{};
		s_Data.TestUniformBuffers->SetData(frameIndex, &transforms, sizeof(TestTransforms));

		s_RendererAPI->BeginRenderPass(s_Data.TestRenderPass);
		s_RendererAPI->DrawIndexed(s_Data.TestRenderPass, s_Data.TestResourceManager, s_Data.ClearVertex, s_Data.ClearIndices);
		s_RendererAPI->EndRenderPass();
	}

	void Renderer::AddNewQuadBatch()
	{
		uint32_t framesInFlight = GetFramesInFlight();

		auto& newVertexBuffers = s_Data.QuadVertexBuffers.emplace_back();
		newVertexBuffers.resize(framesInFlight);
		for (uint32_t frame = 0; frame < framesInFlight; frame++)
		{
			newVertexBuffers[frame] = VertexBuffer::Create(s_Data.MaxQuadVerticesPerBatch * sizeof(QuadVertex));
		}

		auto& newBatch = s_Data.QuadBatchStarts.emplace_back();
		newBatch = znew QuadVertex[s_Data.MaxQuadVerticesPerBatch];
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
		s_Data.BatchIndex = s_Data.QuadIndexCount / s_Data.MaxQuadIndicesPerBatch;

		if (s_Data.BatchIndex >= s_Data.QuadBatchStarts.size())
		{
			AddNewQuadBatch();
			s_Data.QuadBatchEnds.emplace_back();
			s_Data.QuadBatchEnds[s_Data.BatchIndex] = s_Data.QuadBatchStarts[s_Data.BatchIndex];
		}

		auto& newVertex = s_Data.QuadBatchEnds[s_Data.BatchIndex];
		for (int i = 0; i < 4; i++)
		{
			newVertex->Position = transform * s_Data.QuadPositions[i];
			newVertex->Tint = colour;
			newVertex->TextureCoord = s_Data.QuadTextureCoords[i];
			/*newVertex->TextureIndex = 0.0f;
			newVertex->TilingFactor = 1.0f;
			newVertex->EntityID = entityID;*/

			newVertex++;
		}

		s_Data.QuadIndexCount += 6;
		s_Data.Stats.QuadCount++;
	}

	void Renderer::DrawQuad(const glm::mat4& transform, const Ref<Texture2D> texture, const glm::vec4& tint, float tiling, int entityID)
	{
		//// TODO: separate maxima for each primitive type
		//if (s_Data.QuadIndexCount >= RendererData::MaxQuadIndicesPerBatch)
		//{
		//	SubmitCurrentQuadBatch();
		//	AddNewQuadBatch();
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
		//			AddNewQuadBatch();
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
		//if (s_Data.CircleIndexCount >= RendererData::MaxQuadIndicesPerBatch)
		//{
		//	SubmitCurrentQuadBatch();
		//	AddNewQuadBatch();
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
		//if (s_Data.LineVertexCount >= RendererData::MaxQuadVerticesPerBatch)
		//{
		//	SubmitCurrentQuadBatch();
		//	AddNewQuadBatch();
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

	const Renderer::Statistics& Renderer::GetStats()
	{
		return s_Data.Stats;
	}

	void Renderer::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Renderer::Statistics));
	}
	

}
