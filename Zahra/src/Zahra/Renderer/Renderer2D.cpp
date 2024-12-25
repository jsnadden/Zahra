#include "zpch.h"
#include "Renderer2D.h"

namespace Zahra
{
	Renderer2D::Renderer2D(Renderer2DSpecification specification)
		:
		m_Specification(specification),
		c_MaxBatchSize(specification.MaxBatchSize),
		c_MaxQuadVerticesPerBatch(4 * specification.MaxBatchSize),
		c_MaxQuadIndicesPerBatch(6 * specification.MaxBatchSize),
		c_MaxLineVerticesPerBatch(2 * specification.MaxBatchSize)
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
		// SHADERS
		{
			ShaderSpecification shaderSpec{};
			shaderSpec.Name = "flat_colour";
			m_ShaderLibrary.Add(Shader::Create(shaderSpec));
			shaderSpec.Name = "flat_texture";
			m_ShaderLibrary.Add(Shader::Create(shaderSpec));
			shaderSpec.Name = "circle";
			m_ShaderLibrary.Add(Shader::Create(shaderSpec));
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CAMERA DATA
		{
			m_CameraUniformBufferSet = UniformBufferSet::Create(sizeof(glm::mat4), framesInFlight);

			/*glm::mat4 identityMat(1.0f);

			for (uint32_t frame = 0; frame < framesInFlight; frame++)
				m_CameraUniformBufferSet->SetData(frame, &identityMat, sizeof(CameraData));*/
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TEXTURE SLOTS
		{
			m_TextureSlots.resize(m_Specification.MaxTextureSlots);

			// TODO: setup Shader reflection and ShaderResourceManager to be capable of sampler arrays
			Texture2DSpecification flatWhite{};
			m_TextureSlots[0] = Texture2D::CreateFlatColourTexture(flatWhite, 0xffffffff);
			//m_TextureSlots[0] = Texture2D::CreateFromFile(flatWhite, "yajirobe.png");
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// QUADS
		{
			m_QuadTemplate[0] = { -.5f, -.5f, .0f, 1.0f };
			m_QuadTemplate[1] = { .5f, -.5f, .0f, 1.0f };
			m_QuadTemplate[2] = { .5f,  .5f, .0f, 1.0f };
			m_QuadTemplate[3] = { -.5f,  .5f, .0f, 1.0f };

			m_TextureTemplate[0] = { 0.0f, 0.0f };
			m_TextureTemplate[1] = { 1.0f, 0.0f };
			m_TextureTemplate[2] = { 1.0f, 1.0f };
			m_TextureTemplate[3] = { 0.0f, 1.0f };

			/*ShaderSpecification shaderSpec{};
			shaderSpec.Name = "flat_texture";
			m_QuadShader = Shader::Create(shaderSpec);*/
			// TODO: obtain Shaders from a Shaderlibrary instead of directly constructing them here

			// TODO: put a resource manager in renderpass, specifically for
			// camera and light resources (give these a specific set range)
			// For other resources, the manager should belong to a material
			ShaderResourceManagerSpecification resourceManagerSpec{};
			resourceManagerSpec.Shader = m_ShaderLibrary.Get("flat_texture");
			resourceManagerSpec.FirstSet = 0;
			resourceManagerSpec.LastSet = 0;
			m_QuadResourceManager = ShaderResourceManager::Create(resourceManagerSpec);

			m_QuadResourceManager->ProvideResource("Camera", m_CameraUniformBufferSet);
			m_QuadResourceManager->ProvideResource("u_Sampler", m_TextureSlots[0]);
			m_QuadResourceManager->Bake();

			RenderPassSpecification renderPassSpec{};
			renderPassSpec.Name = "quad_pass";
			renderPassSpec.Shader = m_ShaderLibrary.Get("flat_texture");
			renderPassSpec.RenderTarget = m_Specification.RenderTarget;
			renderPassSpec.Topology = PrimitiveTopology::Triangles;
			renderPassSpec.BackfaceCulling = false;
			m_QuadRenderPass = RenderPass::Create(renderPassSpec);

			m_QuadVertexBuffers.resize(1);
			m_QuadVertexBuffers[0].resize(framesInFlight);
			for (uint32_t frame = 0; frame < framesInFlight; frame++)
			{
				m_QuadVertexBuffers[0][frame] = VertexBuffer::Create(c_MaxQuadVerticesPerBatch * sizeof(QuadVertex));
			}

			m_QuadBatchStarts.resize(1);
			m_QuadBatchEnds.resize(1);
			m_QuadBatchStarts[0] = znew QuadVertex[c_MaxQuadVerticesPerBatch];

			uint32_t* indices = znew uint32_t[c_MaxQuadIndicesPerBatch];
			uint32_t offset = 0;
			for (uint32_t i = 0; i < c_MaxQuadIndicesPerBatch; i += 6)
			{
				indices[i + 0] = offset + 0;
				indices[i + 1] = offset + 1;
				indices[i + 2] = offset + 2;

				indices[i + 3] = offset + 2;
				indices[i + 4] = offset + 3;
				indices[i + 5] = offset + 0;

				offset += 4;
			}
			m_QuadIndexBuffer = IndexBuffer::Create(indices, c_MaxQuadIndicesPerBatch);
			delete[] indices;
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLES
		{
			/*ShaderSpecification shaderSpec{};
			shaderSpec.Name = "circle";
			m_CircleShader = Shader::Create(shaderSpec);*/

			ShaderResourceManagerSpecification resourceManagerSpec{};
			resourceManagerSpec.Shader = m_ShaderLibrary.Get("circle");
			resourceManagerSpec.FirstSet = 0;
			resourceManagerSpec.LastSet = 0;
			m_CircleResourceManager = ShaderResourceManager::Create(resourceManagerSpec);

			m_CircleResourceManager->ProvideResource("Camera", m_CameraUniformBufferSet);
			m_CircleResourceManager->Bake();

			RenderPassSpecification renderPassSpec{};
			renderPassSpec.Name = "circle_pass";
			renderPassSpec.Shader = m_ShaderLibrary.Get("circle");
			renderPassSpec.RenderTarget = m_QuadRenderPass->GetRenderTarget();
			renderPassSpec.Topology = PrimitiveTopology::Triangles;
			renderPassSpec.BackfaceCulling = false;
			m_CircleRenderPass = RenderPass::Create(renderPassSpec);

			m_CircleVertexBuffers.resize(1);
			m_CircleVertexBuffers[0].resize(framesInFlight);
			for (uint32_t frame = 0; frame < framesInFlight; frame++)
			{
				m_CircleVertexBuffers[0][frame] = VertexBuffer::Create(c_MaxQuadVerticesPerBatch * sizeof(CircleVertex));
			}

			m_CircleBatchStarts.resize(1);
			m_CircleBatchEnds.resize(1);
			m_CircleBatchStarts[0] = znew CircleVertex[c_MaxQuadVerticesPerBatch];
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// LINES
		{
			/*ShaderSpecification shaderSpec{};
			shaderSpec.Name = "flat_colour";
			m_LineShader = Shader::Create(shaderSpec);*/

			ShaderResourceManagerSpecification resourceManagerSpec{};
			resourceManagerSpec.Shader = m_ShaderLibrary.Get("flat_colour");
			resourceManagerSpec.FirstSet = 0;
			resourceManagerSpec.LastSet = 0;
			m_LineResourceManager = ShaderResourceManager::Create(resourceManagerSpec);

			m_LineResourceManager->ProvideResource("Camera", m_CameraUniformBufferSet);
			m_LineResourceManager->Bake();

			RenderPassSpecification renderPassSpec{};
			renderPassSpec.Name = "line_pass";
			renderPassSpec.Shader = m_ShaderLibrary.Get("flat_colour");
			renderPassSpec.RenderTarget = m_CircleRenderPass->GetRenderTarget();
			renderPassSpec.Topology = PrimitiveTopology::Lines;
			renderPassSpec.DynamicLineWidths = true;
			m_LineRenderPass = RenderPass::Create(renderPassSpec);

			m_LineVertexBuffers.resize(1);
			m_LineVertexBuffers[0].resize(framesInFlight);
			for (uint32_t frame = 0; frame < framesInFlight; frame++)
			{
				m_LineVertexBuffers[0][frame] = VertexBuffer::Create(c_MaxLineVerticesPerBatch * sizeof(LineVertex));
			}

			m_LineBatchStarts.resize(1);
			m_LineBatchEnds.resize(1);
			m_LineBatchStarts[0] = znew LineVertex[c_MaxLineVerticesPerBatch];
		}
	}

	void Renderer2D::Shutdown()
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// LINES
		{
			m_LineBatchEnds.clear();
			for (auto batch : m_LineBatchStarts)
			{
				zdelete[] batch;
			}
			m_LineBatchStarts.clear();

			uint32_t framesInFlight = Renderer::GetFramesInFlight();
			for (auto& batch : m_LineVertexBuffers)
			{
				for (uint32_t frame = 0; frame < framesInFlight; frame++)
				{
					batch[frame].Reset();
				}
				batch.clear();
			}
			m_LineVertexBuffers.clear();

			m_LineRenderPass.Reset();
			m_LineResourceManager.Reset();
			//m_LineShader.Reset();
		}
		
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLES
		{
			m_CircleBatchEnds.clear();
			for (auto batch : m_CircleBatchStarts)
			{
				zdelete[] batch;
			}
			m_CircleBatchStarts.clear();

			uint32_t framesInFlight = Renderer::GetFramesInFlight();
			for (auto& batch : m_CircleVertexBuffers)
			{
				for (uint32_t frame = 0; frame < framesInFlight; frame++)
				{
					batch[frame].Reset();
				}
				batch.clear();
			}
			m_CircleVertexBuffers.clear();

			m_CircleRenderPass.Reset();
			m_CircleResourceManager.Reset();
			//m_CircleShader.Reset();
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
			//m_QuadShader.Reset();
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

	void Renderer2D::BeginScene(const glm::mat4& cameraPV)
	{
		ResetStats();

		m_CameraUniformBufferSet->SetData(Renderer::GetCurrentFrameIndex(), &cameraPV, sizeof(glm::mat4));

		// TODO: reset texture array

		m_QuadIndexCount = 0;
		for (uint32_t batch = 0; batch < m_QuadBatchEnds.size(); batch++)
			m_QuadBatchEnds[batch] = m_QuadBatchStarts[batch];

		m_CircleIndexCount = 0;
		for (uint32_t batch = 0; batch < m_CircleBatchEnds.size(); batch++)
			m_CircleBatchEnds[batch] = m_CircleBatchStarts[batch];

		m_LineVertexCount = 0;
		for (uint32_t batch = 0; batch < m_LineBatchEnds.size(); batch++)
			m_LineBatchEnds[batch] = m_LineBatchStarts[batch];
	}

	void Renderer2D::EndScene()
	{
		uint32_t frame = Renderer::GetCurrentFrameIndex();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// QUAD PASS
		Renderer::BeginRenderPass(m_QuadRenderPass);
		{
			for (uint32_t batch = 0; batch <= m_LastQuadBatch; batch++)
			{
				uint32_t dataSize = (uint32_t)((byte*)m_QuadBatchEnds[batch] - (byte*)m_QuadBatchStarts[batch]);
				if (dataSize)
				{
					uint32_t batchSize = (batch == m_LastQuadBatch) ?
						m_QuadIndexCount - batch * c_MaxQuadIndicesPerBatch :
						c_MaxQuadIndicesPerBatch;

					m_QuadVertexBuffers[batch][frame]->SetData(m_QuadBatchStarts[batch], dataSize);

					// TODO: provide texture array
					m_QuadResourceManager->ProvideResource("u_Sampler", m_TextureSlots[0]);

					Renderer::DrawIndexed(m_QuadRenderPass, m_QuadResourceManager, m_QuadVertexBuffers[batch][frame], m_QuadIndexBuffer, batchSize);

					m_Stats.QuadBatchCount++;
					m_Stats.DrawCalls++;
				}
			}
		}
		Renderer::EndRenderPass();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLE PASS
		Renderer::BeginRenderPass(m_CircleRenderPass);
		{
			for (uint32_t batch = 0; batch <= m_LastCircleBatch; batch++)
			{
				uint32_t dataSize = (uint32_t)((byte*)m_CircleBatchEnds[batch] - (byte*)m_CircleBatchStarts[batch]);
				if (dataSize)
				{
					uint32_t batchSize = (batch == m_LastCircleBatch) ?
						m_CircleIndexCount - batch * c_MaxQuadIndicesPerBatch :
						c_MaxQuadIndicesPerBatch;

					m_CircleVertexBuffers[batch][frame]->SetData(m_CircleBatchStarts[batch], dataSize);

					Renderer::DrawIndexed(m_CircleRenderPass, m_CircleResourceManager, m_CircleVertexBuffers[batch][frame], m_QuadIndexBuffer, batchSize);

					m_Stats.CircleBatchCount++;
					m_Stats.DrawCalls++;
				}
			}
		}
		Renderer::EndRenderPass();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// LINE PASS
		Renderer::BeginRenderPass(m_LineRenderPass);
		{
			Renderer::SetLineWidth(m_LineWidth);

			for (uint32_t batch = 0; batch <= m_LastLineBatch; batch++)
			{
				uint32_t dataSize = (uint32_t)((byte*)m_LineBatchEnds[batch] - (byte*)m_LineBatchStarts[batch]);
				if (dataSize)
				{
					uint32_t batchSize = (batch == m_LastLineBatch) ?
						m_LineVertexCount - batch * c_MaxLineVerticesPerBatch :
						c_MaxLineVerticesPerBatch;

					m_LineVertexBuffers[batch][frame]->SetData(m_LineBatchStarts[batch], dataSize);

					Renderer::Draw(m_LineRenderPass, m_LineResourceManager, m_LineVertexBuffers[batch][frame], batchSize);

					m_Stats.LineBatchCount++;
					m_Stats.DrawCalls++;
				}
			}
		}
		Renderer::EndRenderPass();
	}

	void Renderer2D::AddNewQuadBatch()
	{
		uint32_t framesInFlight = Renderer::GetFramesInFlight();

		auto& newVertexBufferPerFrame = m_QuadVertexBuffers.emplace_back();
		newVertexBufferPerFrame.resize(framesInFlight);
		for (uint32_t frame = 0; frame < framesInFlight; frame++)
		{
			newVertexBufferPerFrame[frame] = VertexBuffer::Create(c_MaxQuadVerticesPerBatch * sizeof(QuadVertex));
		}

		auto& newBatch = m_QuadBatchStarts.emplace_back();
		newBatch = znew QuadVertex[c_MaxQuadVerticesPerBatch];
	}

	void Renderer2D::AddNewCircleBatch()
	{
		uint32_t framesInFlight = Renderer::GetFramesInFlight();

		auto& newVertexBufferPerFrame = m_CircleVertexBuffers.emplace_back();
		newVertexBufferPerFrame.resize(framesInFlight);
		for (uint32_t frame = 0; frame < framesInFlight; frame++)
		{
			newVertexBufferPerFrame[frame] = VertexBuffer::Create(c_MaxQuadVerticesPerBatch * sizeof(CircleVertex));
		}

		auto& newBatch = m_CircleBatchStarts.emplace_back();
		newBatch = znew CircleVertex[c_MaxQuadVerticesPerBatch];
	}

	void Renderer2D::AddNewLineBatch()
	{
		uint32_t framesInFlight = Renderer::GetFramesInFlight();

		auto& newVertexBufferPerFrame = m_LineVertexBuffers.emplace_back();
		newVertexBufferPerFrame.resize(framesInFlight);
		for (uint32_t frame = 0; frame < framesInFlight; frame++)
		{
			newVertexBufferPerFrame[frame] = VertexBuffer::Create(c_MaxLineVerticesPerBatch * sizeof(LineVertex));
		}

		auto& newBatch = m_LineBatchStarts.emplace_back();
		newBatch = znew LineVertex[c_MaxLineVerticesPerBatch];
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& colour, int entityID)
	{
		m_LastQuadBatch = m_QuadIndexCount / c_MaxQuadIndicesPerBatch;

		if (m_LastQuadBatch >= m_QuadBatchStarts.size())
		{
			AddNewQuadBatch();
			m_QuadBatchEnds.emplace_back();
			m_QuadBatchEnds[m_LastQuadBatch] = m_QuadBatchStarts[m_LastQuadBatch];
		}

		auto& newVertex = m_QuadBatchEnds[m_LastQuadBatch];
		for (int i = 0; i < 4; i++)
		{
			newVertex->Position = transform * m_QuadTemplate[i];
			newVertex->Tint = colour;
			newVertex->TextureCoord = m_TextureTemplate[i];
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
		m_LastCircleBatch = m_CircleIndexCount / c_MaxQuadIndicesPerBatch;

		if (m_LastCircleBatch >= m_CircleBatchStarts.size())
		{
			AddNewCircleBatch();
			m_CircleBatchEnds.emplace_back();
			m_CircleBatchEnds[m_LastCircleBatch] = m_CircleBatchStarts[m_LastCircleBatch];
		}

		auto& newVertex = m_CircleBatchEnds[m_LastCircleBatch];
		for (int i = 0; i < 4; i++)
		{
			newVertex->WorldPosition = transform * m_QuadTemplate[i];
			newVertex->LocalPosition = m_QuadTemplate[i] * 2.0f;
			newVertex->Colour = colour;
			newVertex->Thickness = thickness;
			newVertex->Fade = fade;
			//newVertex->EntityID = entityID;

			newVertex++;
		}

		m_CircleIndexCount += 6;
		m_Stats.CircleCount++;
	}

	void Renderer2D::DrawLine(const glm::vec3& end0, const glm::vec3& end1, const glm::vec4& colour, int entityID)
	{
		m_LastLineBatch = m_LineVertexCount / c_MaxLineVerticesPerBatch;

		if (m_LastLineBatch >= m_LineBatchStarts.size())
		{
			AddNewLineBatch();
			m_LineBatchEnds.emplace_back();
			m_LineBatchEnds[m_LastLineBatch] = m_LineBatchStarts[m_LastLineBatch];
		}

		auto& newVertex = m_LineBatchEnds[m_LastLineBatch];

		newVertex->Position = end0;
		newVertex->Colour = colour;
		//newVertex->EntityID = entityID;
		newVertex++;
		
		newVertex->Position = end1;
		newVertex->Colour = colour;
		//newVertex->EntityID = entityID;
		newVertex++;
		
		m_LineVertexCount += 2;
		m_Stats.LineCount++;
	}

	void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& colour, int entityID)
	{
		for (int i = 0; i < 4; i++)
			DrawLine(transform * m_QuadTemplate[i], transform * m_QuadTemplate[(i + 1) % 4], colour, entityID);
	}

	void Renderer2D::OnWindowResize(uint32_t width, uint32_t height)
	{
		m_QuadRenderPass->OnResize();
		m_CircleRenderPass->OnResize();
	}
}
