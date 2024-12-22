#include "zpch.h"
#include "Renderer2D.h"

namespace Zahra
{
	Renderer2D::Renderer2D(Renderer2DSpecification specification)
		:
		m_Specification(specification),
		c_MaxBatchSize(specification.MaxBatchSize),
		c_MaxQuadVerticesPerBatch(4 * specification.MaxBatchSize),
		c_MaxQuadIndicesPerBatch(6 * specification.MaxBatchSize)
	{
		Z_CORE_ASSERT(m_Specification.RenderTarget, "Must provide a target framebuffer in the Renderer2DSpecification");

		Init();
	}

	Renderer2D::~Renderer2D()
	{
		Shutdown();
	}

	void Renderer2D::Init()
	{
		uint32_t framesInFlight = Renderer::GetFramesInFlight();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA DATA
		{
			m_CameraUniformBufferSet = UniformBufferSet::Create(sizeof(CameraData), framesInFlight);

			for (uint32_t frame = 0; frame < framesInFlight; frame++)
				m_CameraUniformBufferSet->SetData(frame, &m_CameraData, sizeof(CameraData));
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TEXTURE SLOTS
		{
			m_TextureSlots.resize(m_Specification.MaxTextureSlots);

			Texture2DSpecification flatWhite{};
			m_TextureSlots[0] = Texture2D::CreateFlatColourTexture(flatWhite, 0xffffffff);
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// QUADS
		{
			// TODO: setup Shader reflection and ShaderResourceManager to be capable of using arrays
			// of textures... until that's set up we'll just use slot 0 (a single pure white pixel)

			ShaderSpecification quadShaderSpec{};
			quadShaderSpec.Name = "quad";
			m_QuadShader = Shader::Create(quadShaderSpec);
			// TODO: obtain Shaders from a Shaderlibrary instead of directly constructing them here

			ShaderResourceManagerSpecification quadResourceManagerSpec{};
			quadResourceManagerSpec.Shader = m_QuadShader;
			quadResourceManagerSpec.FirstSet = 0;
			quadResourceManagerSpec.LastSet = 0;
			m_QuadResourceManager = ShaderResourceManager::Create(quadResourceManagerSpec);

			m_QuadResourceManager->ProvideResource("Camera", m_CameraUniformBufferSet);
			m_QuadResourceManager->ProvideResource("u_Sampler", m_TextureSlots[0]);
			m_QuadResourceManager->Bake();

			RenderPassSpecification quadRenderPassSpec{};
			quadRenderPassSpec.Name = "quad_pass";
			quadRenderPassSpec.Shader = m_QuadShader;
			quadRenderPassSpec.RenderTarget = Renderer::GetLoadPassFramebuffer();
			quadRenderPassSpec.Topology = PrimitiveTopology::Triangles;
			quadRenderPassSpec.BackfaceCulling = false;
			m_QuadRenderPass = RenderPass::Create(quadRenderPassSpec);

			m_QuadVertexBuffers.resize(1);
			m_QuadVertexBuffers[0].resize(framesInFlight);
			for (uint32_t frame = 0; frame < framesInFlight; frame++)
			{
				m_QuadVertexBuffers[0][frame] = VertexBuffer::Create(c_MaxQuadVerticesPerBatch * sizeof(QuadVertex));
			}

			m_QuadBatchStarts.resize(1);
			m_QuadBatchEnds.resize(1);
			m_QuadBatchStarts[0] = znew QuadVertex[c_MaxQuadVerticesPerBatch];

			uint32_t* quadIndices = znew uint32_t[c_MaxQuadIndicesPerBatch];
			uint32_t offset = 0;
			for (uint32_t i = 0; i < c_MaxQuadIndicesPerBatch; i += 6)
			{
				quadIndices[i + 0] = offset + 0;
				quadIndices[i + 1] = offset + 1;
				quadIndices[i + 2] = offset + 2;

				quadIndices[i + 3] = offset + 2;
				quadIndices[i + 4] = offset + 3;
				quadIndices[i + 5] = offset + 0;

				offset += 4;
			}
			m_QuadIndexBuffer = IndexBuffer::Create(quadIndices, c_MaxQuadIndicesPerBatch);
			delete[] quadIndices;

			m_QuadIndexCount = 0;

			m_QuadPositions[0] = { -.5f, -.5f, .0f, 1.0f };
			m_QuadPositions[1] = { .5f, -.5f, .0f, 1.0f };
			m_QuadPositions[2] = { .5f,  .5f, .0f, 1.0f };
			m_QuadPositions[3] = { -.5f,  .5f, .0f, 1.0f };

			m_QuadTextureCoords[0] = { 0.0f, 0.0f };
			m_QuadTextureCoords[1] = { 1.0f, 0.0f };
			m_QuadTextureCoords[2] = { 1.0f, 1.0f };
			m_QuadTextureCoords[3] = { 0.0f, 1.0f };
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
	}

	void Renderer2D::Shutdown()
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// LINES
		{
			// TODO:
		}
		
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLES
		{
			// TODO:
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// QUADS
		{
			m_QuadIndexBuffer.Reset();

			m_QuadBatchEnds.clear();
			for (auto batch : m_QuadBatchStarts)
			{
				zdelete[] batch;
			}
			m_QuadBatchStarts.clear();

			uint32_t framesInFlight = Renderer::GetFramesInFlight();
			for (auto& batch : m_QuadVertexBuffers)
			{
				for (uint32_t frame = 0; frame < framesInFlight; frame++)
				{
					batch[frame].Reset();
				}
				batch.clear();
			}
			m_QuadVertexBuffers.clear();

			m_QuadRenderPass.Reset();
			m_QuadResourceManager.Reset();
			m_QuadShader.Reset();
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TEXTURE SLOTS
		{
			for (auto& texture : m_TextureSlots)
			{
				texture.Reset();
			}
			m_TextureSlots.clear();
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA DATA
		{
			m_CameraUniformBufferSet.Reset();
		}
	}

	void Renderer2D::BeginScene(const glm::mat4& view, const glm::mat4& projection)
	{
		uint32_t frame = Renderer::GetCurrentFrameIndex();

		m_CameraData.View = view;
		m_CameraData.Projection = projection;
		m_CameraUniformBufferSet->SetData(frame, &m_CameraData, sizeof(CameraData));

		m_QuadIndexCount = 0;

		for (uint32_t batch = 0; batch < m_QuadBatchEnds.size(); batch++)
		{
			m_QuadBatchEnds[batch] = m_QuadBatchStarts[batch];
		}

		// TODO: reset texture array
	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		/*m_CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		m_CameraUniformBuffer->SetData(&m_CameraBuffer, sizeof(RendererData::CameraData));*/
	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		/*m_CameraBuffer.ViewProjection = camera.GetPVMatrix();
		m_CameraUniformBuffer->SetData(&m_CameraBuffer, sizeof(RendererData::CameraData));*/
	}

	void Renderer2D::EndScene()
	{
		uint32_t frame = Renderer::GetCurrentFrameIndex();

		Renderer::BeginRenderPass(m_QuadRenderPass);
		{
			for (uint32_t batch = 0; batch <= m_BatchIndex; batch++)
			{
				uint32_t dataSize = (uint32_t)((byte*)m_QuadBatchEnds[batch] - (byte*)m_QuadBatchStarts[batch]);
				if (dataSize)
				{
					uint32_t batchSize = (batch == m_BatchIndex) ?
						m_QuadIndexCount - batch * c_MaxQuadIndicesPerBatch :
						c_MaxQuadIndicesPerBatch;

					m_QuadVertexBuffers[batch][frame]->SetData(m_QuadBatchStarts[batch], dataSize);

					// TODO: assign ALL textures in array
					m_QuadResourceManager->ProvideResource("u_Sampler", m_TextureSlots[0]);

					// TODO: compare this to moving the begin/end render pass outside of the for loop

					Renderer::DrawIndexed(m_QuadRenderPass, m_QuadResourceManager, m_QuadVertexBuffers[batch][frame], m_QuadIndexBuffer, batchSize);

					m_Stats.DrawCalls++;
				}
			}
		}
		Renderer::EndRenderPass();
	}

	void Renderer2D::AddNewQuadBatch()
	{
		uint32_t framesInFlight = Renderer::GetFramesInFlight();

		auto& newVertexBuffers = m_QuadVertexBuffers.emplace_back();
		newVertexBuffers.resize(framesInFlight);
		for (uint32_t frame = 0; frame < framesInFlight; frame++)
		{
			newVertexBuffers[frame] = VertexBuffer::Create(c_MaxQuadVerticesPerBatch * sizeof(QuadVertex));
		}

		auto& newBatch = m_QuadBatchStarts.emplace_back();
		newBatch = znew QuadVertex[c_MaxQuadVerticesPerBatch];
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& colour, int entityID)
	{
		m_BatchIndex = m_QuadIndexCount / c_MaxQuadIndicesPerBatch;

		if (m_BatchIndex >= m_QuadBatchStarts.size())
		{
			AddNewQuadBatch();
			m_QuadBatchEnds.emplace_back();
			m_QuadBatchEnds[m_BatchIndex] = m_QuadBatchStarts[m_BatchIndex];
		}

		auto& newVertex = m_QuadBatchEnds[m_BatchIndex];
		for (int i = 0; i < 4; i++)
		{
			newVertex->Position = transform * m_QuadPositions[i];
			newVertex->Tint = colour;
			newVertex->TextureCoord = m_QuadTextureCoords[i];
			/*newVertex->TextureIndex = 0.0f;
			newVertex->TilingFactor = 1.0f;
			newVertex->EntityID = entityID;*/

			newVertex++;
		}

		m_QuadIndexCount += 6;
		m_Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D> texture, const glm::vec4& tint, float tiling, int entityID)
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

	void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& colour, float thickness, float fade, int entityID)
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

	void Renderer2D::DrawLine(const glm::vec3& end0, const glm::vec3& end1, const glm::vec4& colour, int entityID)
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

	void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& colour, int entityID)
	{
		/*glm::vec3 corners[4] = { {.5f, .5f, .0f}, {-.5f, .5f, .0f}, {-.5f, -.5f, .0f}, {.5f, -.5f, .0f} };

		for (int i = 0; i < 4; i++)
			DrawLine(transform * glm::vec4(corners[i], 1.f), transform * glm::vec4(corners[(i+1)%4], 1.f), colour, entityID);*/
	}

	void Renderer2D::OnWindowResize(uint32_t width, uint32_t height)
	{
		m_QuadRenderPass->OnResize();
	}
}
