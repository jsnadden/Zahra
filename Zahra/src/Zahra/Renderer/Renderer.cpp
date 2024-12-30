#include "zpch.h"
#include "Renderer.h"

#include "Zahra/Core/Types.h"
#include "Zahra/Renderer/IndexBuffer.h"
#include "Zahra/Renderer/Mesh.h"
#include "Zahra/Renderer/RenderPass.h"
#include "Zahra/Renderer/Shader.h"
#include "Zahra/Renderer/ShaderResourceManager.h"
#include "Zahra/Renderer/UniformBuffer.h"
#include "Zahra/Renderer/VertexBuffer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace Zahra
{
	struct TestTransforms
	{
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 View = glm::mat4(1.0f);
		glm::mat4 Projection = glm::mat4(1.0f);
	};

	struct RendererData
	{
		RendererConfig					Config;
		bool							ConfigSet = false;

		Renderer::Capabilities			Capabilities;
		Renderer::Statistics			Statistics;

		Ref<Image2D>					PrimaryRenderTargetImage;
		Ref<Texture2D>					PrimaryRenderTargetTexture;
		Ref<Framebuffer>				PrimaryFramebuffer;
		Ref<RenderPass>					ClearRenderPass;

		Ref<Shader>						FullscreenTriangleShader;
		Ref<ShaderResourceManager>		FullscreenTriangleResourceManager;
		Ref<RenderPass>					FullscreenTriangleRenderPass;
		
		Ref<Shader>						TestSceneShader;
		Ref<ShaderResourceManager>		TestSceneResourceManager;
		Ref<RenderPass>					TestSceneRenderPass;
		Ref<StaticMesh>					TestSceneMesh;
		Ref<Texture2D>					TestSceneTexture;
		Ref<UniformBufferSet>			TestSceneUniformBuffers;
	};

	static RendererData s_Data;

	static RendererAPI* s_RendererAPI;

	// TODO: Add a render command queue! Most of the static methods here can simply add the relevant s_RendererAPI
	// call to the queue, but also add a method to allow other classes to enqueue their own commands

	void Renderer::Init()
	{
		s_RendererAPI = RendererAPI::Create();
		s_RendererAPI->Init();

		std::filesystem::path cache = s_Data.Config.ShaderCacheDirectory;
		if (!std::filesystem::exists(cache))
			std::filesystem::create_directories(cache);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// PRIMARY RENDER TARGET
		{
			Image2DSpecification imageSpec{};
			imageSpec.Name = "Renderer_PrimaryRenderTarget";
			imageSpec.Format = ImageFormat::SRGBA;
			imageSpec.Width = s_RendererAPI->GetSwapchainWidth();
			imageSpec.Height = s_RendererAPI->GetSwapchainHeight();
			imageSpec.Sampled = true;
			imageSpec.InitialLayout = ImageLayout::ColourAttachment;
			s_Data.PrimaryRenderTargetImage = Image2D::Create(imageSpec);

			s_Data.PrimaryRenderTargetTexture = Texture2D::CreateFromImage2D(s_Data.PrimaryRenderTargetImage);

			FramebufferSpecification framebufferSpec{};
			framebufferSpec.Name = "Renderer_PrimaryFramebuffer";
			framebufferSpec.Width = s_RendererAPI->GetSwapchainWidth();
			framebufferSpec.Height = s_RendererAPI->GetSwapchainHeight();
			//framebufferSpec.ClearColour = { .0f, .0f, .0f };
			{
				auto& attachment = framebufferSpec.ColourAttachmentSpecs.emplace_back();
				attachment.InheritFrom = s_Data.PrimaryRenderTargetImage;
				attachment.Format = ImageFormat::SRGBA;
			}
			framebufferSpec.HasDepthStencil = true;
			framebufferSpec.DepthClearValue = 1.0f;
			framebufferSpec.DepthStencilAttachmentSpec.Format = ImageFormat::DepthStencil;
			s_Data.PrimaryFramebuffer = Framebuffer::Create(framebufferSpec);

			RenderPassSpecification renderPassSpec{};
			renderPassSpec.Name = "Renderer_ClearRenderPass";
			renderPassSpec.RenderTarget = s_Data.PrimaryFramebuffer;
			renderPassSpec.ClearColourAttachments = true;
			renderPassSpec.ClearDepthAttachment = true;
			s_Data.ClearRenderPass = RenderPass::Create(renderPassSpec);
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// FULLSCREEN TRIANGLE RENDERING
		{
			ShaderSpecification shaderSpec{};
			shaderSpec.Name = "fullscreen_triangle";
			s_Data.FullscreenTriangleShader = Shader::Create(shaderSpec);

			ShaderResourceManagerSpecification resourceManagerSpec{};
			resourceManagerSpec.Shader = s_Data.FullscreenTriangleShader;
			resourceManagerSpec.FirstSet = 0;
			resourceManagerSpec.LastSet = 0;
			s_Data.FullscreenTriangleResourceManager = ShaderResourceManager::Create(resourceManagerSpec);

			RenderPassSpecification renderPassSpec{};
			renderPassSpec.Name = "Renderer_FullscreenTriangleRenderPass";
			renderPassSpec.Shader = s_Data.FullscreenTriangleShader;
			renderPassSpec.ClearColourAttachments = true;
			s_Data.FullscreenTriangleRenderPass = RenderPass::Create(renderPassSpec);

			s_Data.FullscreenTriangleResourceManager->ProvideResource("u_Sampler", s_Data.PrimaryRenderTargetTexture);
			s_Data.FullscreenTriangleResourceManager->Update();
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TEST SCENE
		{
			ShaderSpecification shaderSpec{};
			shaderSpec.Name = "vulkan_tutorial";
			s_Data.TestSceneShader = Shader::Create(shaderSpec);

			ShaderResourceManagerSpecification resourceManagerSpec{};
			resourceManagerSpec.Shader = s_Data.TestSceneShader;
			resourceManagerSpec.FirstSet = 0;
			resourceManagerSpec.LastSet = 0;
			s_Data.TestSceneResourceManager = ShaderResourceManager::Create(resourceManagerSpec);

			RenderPassSpecification renderPassSpec{};
			renderPassSpec.Name = "Renderer_TestSceneRenderPass";
			renderPassSpec.Shader = s_Data.TestSceneShader;
			renderPassSpec.Topology = PrimitiveTopology::Triangles;
			renderPassSpec.RenderTarget = s_Data.PrimaryFramebuffer;
			s_Data.TestSceneRenderPass = RenderPass::Create(renderPassSpec);

			MeshSpecification meshSpec{};
			meshSpec.Name = "viking_room";
			s_Data.TestSceneMesh = StaticMesh::Create(meshSpec);

			uint32_t framesInFlight = s_RendererAPI->GetFramesInFlight();
			uint32_t mvpSize = sizeof(TestTransforms);
			s_Data.TestSceneUniformBuffers = UniformBufferSet::Create(mvpSize, framesInFlight);
			TestTransforms transforms{};
			for (uint32_t frame = 0; frame < framesInFlight; frame++)
				s_Data.TestSceneUniformBuffers->SetData(frame, &transforms, mvpSize);

			Texture2DSpecification tutorialTextureSpec{};
			s_Data.TestSceneTexture = Texture2D::CreateFromFile(tutorialTextureSpec, "Assets/Textures/viking_room.png");

			s_Data.TestSceneResourceManager->ProvideResource("Matrices", s_Data.TestSceneUniformBuffers);
			s_Data.TestSceneResourceManager->ProvideResource("u_Texture", s_Data.TestSceneTexture);
			s_Data.TestSceneResourceManager->Update();
		}

		Z_CORE_INFO("Core rendering engine initialised");
	}

	void Renderer::Shutdown()
	{
		s_Data.TestSceneTexture.Reset();
		s_Data.TestSceneUniformBuffers.Reset();
		s_Data.TestSceneMesh.Reset();
		s_Data.TestSceneRenderPass.Reset();
		s_Data.TestSceneResourceManager.Reset();
		s_Data.TestSceneShader.Reset();

		s_Data.FullscreenTriangleRenderPass.Reset();
		s_Data.FullscreenTriangleResourceManager.Reset();
		s_Data.FullscreenTriangleShader.Reset();

		s_Data.ClearRenderPass.Reset();
		s_Data.PrimaryFramebuffer.Reset();
		s_Data.PrimaryRenderTargetTexture.Reset();
		s_Data.PrimaryRenderTargetImage.Reset();

		s_RendererAPI->Shutdown();
	}

	const RendererConfig& Renderer::GetConfig()
	{
		Z_CORE_ASSERT(s_Data.ConfigSet, "Renderer configuration has not been set");
		return s_Data.Config;
	}

	void Renderer::SetConfig(const RendererConfig& config)
	{
		Z_CORE_ASSERT(!s_Data.ConfigSet, "Renderer configuration already set");
		s_Data.ConfigSet = true;

		s_Data.Config = config;
	}

	Renderer::Capabilities& Renderer::GetCapabilities()
	{
		return s_Data.Capabilities;
	}

	uint32_t Renderer::GetSwapchainWidth()
	{
		return s_RendererAPI->GetSwapchainWidth();
	}

	uint32_t Renderer::GetSwapchainHeight()
	{
		return s_RendererAPI->GetSwapchainHeight();
	}

	uint32_t Renderer::GetFramesInFlight()
	{
		return s_RendererAPI->GetFramesInFlight();
	}

	uint32_t Renderer::GetCurrentFrameIndex()
	{
		return s_RendererAPI->GetCurrentFrameIndex();
	}

	const Ref<Image2D>& Renderer::GetPrimaryRenderTarget()
	{
		return s_Data.PrimaryRenderTargetImage;
	}

	const Ref<Framebuffer>& Renderer::GetPrimaryFramebuffer()
	{
		return s_Data.PrimaryFramebuffer;
	}

	const Renderer::Statistics& Renderer::GetStats()
	{
		return s_Data.Statistics;
	}

	void Renderer::BeginFrame()
	{
		s_RendererAPI->BeginFrame();
		
		uint32_t frameIndex = s_RendererAPI->GetCurrentFrameIndex();
		
		s_RendererAPI->BeginRenderPass(s_Data.ClearRenderPass, false, true);
		s_RendererAPI->EndRenderPass();
	}

	void Renderer::EndFrame()
	{
		s_RendererAPI->BeginRenderPass(s_Data.FullscreenTriangleRenderPass);
		s_RendererAPI->DrawFullscreenTriangle(s_Data.FullscreenTriangleRenderPass, s_Data.FullscreenTriangleResourceManager);
		s_RendererAPI->EndRenderPass();

		s_RendererAPI->EndFrame();
	}

	void Renderer::BeginRenderPass(Ref<RenderPass>& renderPass, bool bindPipeline, bool clearAttachments)
	{
		s_RendererAPI->BeginRenderPass(renderPass, bindPipeline, clearAttachments);
	}

	void Renderer::EndRenderPass()
	{
		s_RendererAPI->EndRenderPass();
	}

	void Renderer::Present()
	{
		s_RendererAPI->Present();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		s_RendererAPI->OnWindowResize();

		s_Data.PrimaryRenderTargetImage->Resize(width, height);
		s_Data.PrimaryRenderTargetTexture->Resize(width, height);
		s_Data.PrimaryFramebuffer->Resize(width, height);
		s_Data.ClearRenderPass->OnResize();

		s_Data.FullscreenTriangleResourceManager->ProvideResource("u_Sampler", s_Data.PrimaryRenderTargetTexture);
		s_Data.FullscreenTriangleResourceManager->Update();
		s_Data.FullscreenTriangleRenderPass->OnResize();

		s_Data.TestSceneRenderPass->OnResize();
	}

	void Renderer::Draw(Ref<RenderPass>& renderPass, Ref<ShaderResourceManager>& resourceManager, Ref<VertexBuffer>& vertexBuffer, uint32_t vertexCount)
	{
		s_RendererAPI->Draw(renderPass, resourceManager, vertexBuffer, vertexCount);

		s_Data.Statistics.DrawCallCount++;
	}

	void Renderer::DrawIndexed(Ref<RenderPass>& renderPass, Ref<ShaderResourceManager>& resourceManager, Ref<VertexBuffer>& vertexBuffer, Ref<IndexBuffer>& indexBuffer, uint32_t indexCount, uint32_t startingIndex)
	{
		s_RendererAPI->DrawIndexed(renderPass, resourceManager, vertexBuffer, indexBuffer, indexCount, startingIndex);

		s_Data.Statistics.DrawCallCount++;
	}

	void Renderer::DrawMesh(Ref<RenderPass>& renderPass, Ref<ShaderResourceManager>& resourceManager, Ref<StaticMesh>& mesh)
	{
		s_RendererAPI->DrawMesh(renderPass, resourceManager, mesh);

		s_Data.Statistics.DrawCallCount++;

	}

	void Renderer::DrawTestScene(glm::mat4 view, glm::mat4 projection)
	{
		float width = (float)GetSwapchainWidth();
		float height = (float)GetSwapchainHeight();
		uint32_t frameIndex = GetCurrentFrameIndex();

		TestTransforms transforms{};
		transforms.Model = glm::rotate(glm::mat4(1.f), glm::radians(-90.f), { 1.f, 0.f, 0.f }) * glm::rotate(glm::mat4(1.f), glm::radians(180.f), { 0.f, 0.f, 1.f });
		transforms.View = view;
		transforms.Projection = projection;

		s_Data.TestSceneUniformBuffers->SetData(frameIndex, &transforms, sizeof(TestTransforms));

		BeginRenderPass(s_Data.TestSceneRenderPass);
		DrawMesh(s_Data.TestSceneRenderPass, s_Data.TestSceneResourceManager, s_Data.TestSceneMesh);
		EndRenderPass();
	}
	void Renderer::SetLineWidth(float width)
	{
		s_RendererAPI->SetLineWidth(width);
	}
}
