#include "zpch.h"
#include "Renderer2D.h"

#include "Zahra/Renderer/Text/MSDF.h"

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

		ShaderSpecification shaderSpec{};
		shaderSpec.Name = "flat_colour";
		m_ShaderLibrary.Add(Shader::Create(shaderSpec));
		shaderSpec.Name = "flat_texture";
		m_ShaderLibrary.Add(Shader::Create(shaderSpec));
		shaderSpec.Name = "circle";
		m_ShaderLibrary.Add(Shader::Create(shaderSpec));
		shaderSpec.Name = "msdf";
		m_ShaderLibrary.Add(Shader::Create(shaderSpec));

		m_CameraUniformBuffers = UniformBufferPerFrame::Create(sizeof(glm::mat4), framesInFlight);

		TextureSpecification textureSpec{};
		auto flatWhite = Texture2D::CreateFlatColourTexture(textureSpec, 0xffffffff);
		m_TextureSlots.resize(m_Specification.MaxTextureSlots, flatWhite);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// QUADS
		{
			m_QuadTemplate[0] = { -.5f, -.5f, .0f, 1.0f };
			m_QuadTemplate[1] = { .5f, -.5f, .0f, 1.0f };
			m_QuadTemplate[2] = { .5f,  .5f, .0f, 1.0f };
			m_QuadTemplate[3] = { -.5f,  .5f, .0f, 1.0f };

			m_TextureTemplate[0] = { 0.0f, 1.0f };
			m_TextureTemplate[1] = { 1.0f, 1.0f };
			m_TextureTemplate[2] = { 1.0f, 0.0f };
			m_TextureTemplate[3] = { 0.0f, 0.0f };

			/*ShaderResourceManagerSpecification resourceManagerSpec{};
			resourceManagerSpec.Shader = m_ShaderLibrary.Get("flat_texture");
			resourceManagerSpec.FirstSet = 0;
			resourceManagerSpec.LastSet = 0;
			m_QuadResourceManager = ShaderResourceManager::Create(resourceManagerSpec);*/

			RenderPassSpecification renderPassSpec{};
			renderPassSpec.Name = "quad_pass";
			renderPassSpec.Shader = m_ShaderLibrary.Get("flat_texture");
			renderPassSpec.RenderTarget = m_Specification.RenderTarget;
			m_QuadRenderPass = RenderPass::Create(renderPassSpec);

			auto quadResourceManager = m_QuadRenderPass->GetResourceManager();
			quadResourceManager->Set("Camera", m_CameraUniformBuffers);
			quadResourceManager->ProcessChanges();

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
			/*ShaderResourceManagerSpecification resourceManagerSpec{};
			resourceManagerSpec.Shader = m_ShaderLibrary.Get("circle");
			resourceManagerSpec.FirstSet = 0;
			resourceManagerSpec.LastSet = 0;
			m_CircleResourceManager = ShaderResourceManager::Create(resourceManagerSpec);*/

			RenderPassSpecification renderPassSpec{};
			renderPassSpec.Name = "circle_pass";
			renderPassSpec.Shader = m_ShaderLibrary.Get("circle");
			renderPassSpec.RenderTarget = m_QuadRenderPass->GetRenderTarget();
			m_CircleRenderPass = RenderPass::Create(renderPassSpec);

			auto circleResourceManager = m_CircleRenderPass->GetResourceManager();
			circleResourceManager->Set("Camera", m_CameraUniformBuffers);
			circleResourceManager->ProcessChanges();

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
			/*ShaderResourceManagerSpecification resourceManagerSpec{};
			resourceManagerSpec.Shader = m_ShaderLibrary.Get("flat_colour");
			resourceManagerSpec.FirstSet = 0;
			resourceManagerSpec.LastSet = 0;
			m_LineResourceManager = ShaderResourceManager::Create(resourceManagerSpec);*/

			RenderPassSpecification renderPassSpec{};
			renderPassSpec.Name = "line_pass";
			renderPassSpec.Shader = m_ShaderLibrary.Get("flat_colour");
			renderPassSpec.Topology = PrimitiveTopology::Lines;
			renderPassSpec.DynamicLineWidths = true;
			renderPassSpec.RenderTarget = m_CircleRenderPass->GetRenderTarget();
			m_LineRenderPass = RenderPass::Create(renderPassSpec);

			auto lineResourceManager = m_LineRenderPass->GetResourceManager();
			lineResourceManager->Set("Camera", m_CameraUniformBuffers);
			lineResourceManager->ProcessChanges();

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

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TEXT
		{
			RenderPassSpecification renderPassSpec{};
			renderPassSpec.Name = "text_pass";
			renderPassSpec.Shader = m_ShaderLibrary.Get("msdf");
			renderPassSpec.RenderTarget = m_Specification.RenderTarget;
			m_TextRenderPass = RenderPass::Create(renderPassSpec);

			auto textResourceManager = m_TextRenderPass->GetResourceManager();
			textResourceManager->Set("Camera", m_CameraUniformBuffers);
			textResourceManager->ProcessChanges();

			m_TextVertexBuffers.resize(1);
			m_TextVertexBuffers[0].resize(framesInFlight);
			for (uint32_t frame = 0; frame < framesInFlight; frame++)
			{
				m_TextVertexBuffers[0][frame] = VertexBuffer::Create(c_MaxQuadVerticesPerBatch * sizeof(TextVertex));
			}

			m_TextBatchStarts.resize(1);
			m_TextBatchEnds.resize(1);
			m_TextBatchStarts[0] = znew TextVertex[c_MaxQuadVerticesPerBatch];
		}
	}

	void Renderer2D::Shutdown()
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TEXT
		{
			for (auto& font : m_Fonts)
			{
				font.Reset();
			}
			m_Fonts.clear();

			m_TextBatchEnds.clear();
			for (auto batch : m_TextBatchStarts)
			{
				zdelete[] batch;
			}
			m_TextBatchStarts.clear();

			uint32_t framesInFlight = Renderer::GetFramesInFlight();
			for (auto& batch : m_TextVertexBuffers)
			{
				for (uint32_t frame = 0; frame < framesInFlight; frame++)
				{
					batch[frame].Reset();
				}
				batch.clear();
			}
			m_TextVertexBuffers.clear();

			m_TextRenderPass.Reset();
		}

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
			//m_LineResourceManager.Reset();
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
			//m_CircleResourceManager.Reset();
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
			//m_QuadResourceManager.Reset();
		}

		for (auto& texture : m_TextureSlots)
		{
			texture.Reset();
		}
		m_TextureSlots.clear();

		m_CameraUniformBuffers.Reset();
	}

	void Renderer2D::BeginScene(const glm::mat4& cameraView, const glm::mat4& cameraProjection)
	{
		glm::mat4 cameraPV = cameraProjection * cameraView;

		m_CameraUniformBuffers->SetData(Renderer::GetCurrentFrameIndex(), &cameraPV, sizeof(glm::mat4));

		m_TextureSlotsInUse = 1;
		for (uint32_t i = 1; i < m_TextureSlots.size(); i++)
			m_TextureSlots[i] = m_TextureSlots[0];

		m_QuadIndexCount = 0;
		for (uint32_t batch = 0; batch < m_QuadBatchEnds.size(); batch++)
			m_QuadBatchEnds[batch] = m_QuadBatchStarts[batch];

		m_CircleIndexCount = 0;
		for (uint32_t batch = 0; batch < m_CircleBatchEnds.size(); batch++)
			m_CircleBatchEnds[batch] = m_CircleBatchStarts[batch];

		m_LineVertexCount = 0;
		for (uint32_t batch = 0; batch < m_LineBatchEnds.size(); batch++)
			m_LineBatchEnds[batch] = m_LineBatchStarts[batch];

		m_Fonts.clear();
		for (uint32_t batch = 0; batch < m_TextBatchEnds.size(); batch++)
			m_TextBatchEnds[batch] = m_TextBatchStarts[batch];
	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		BeginScene(camera.GetView(), camera.GetProjection());
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

					auto quadResourceManager = m_QuadRenderPass->GetResourceManager();
					quadResourceManager->Update("u_Sampler", m_TextureSlots);
					Z_CORE_ASSERT(quadResourceManager->ReadyToRender());
					quadResourceManager->ProcessChanges();

					Renderer::DrawIndexed(m_QuadRenderPass, m_QuadVertexBuffers[batch][frame], m_QuadIndexBuffer, batchSize);

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

					Z_CORE_ASSERT(m_CircleRenderPass->GetResourceManager()->ReadyToRender());

					Renderer::DrawIndexed(m_CircleRenderPass, m_CircleVertexBuffers[batch][frame], m_QuadIndexBuffer, batchSize);

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

					Z_CORE_ASSERT(m_LineRenderPass->GetResourceManager()->ReadyToRender());

					Renderer::Draw(m_LineRenderPass, m_LineVertexBuffers[batch][frame], batchSize);

					m_Stats.LineBatchCount++;
					m_Stats.DrawCalls++;
				}
			}
		}
		Renderer::EndRenderPass();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TEXT PASS
		Renderer::BeginRenderPass(m_TextRenderPass);
		{
			for (uint32_t batch = 0; batch < m_Fonts.size(); batch++)
			{
				uint32_t dataSize = (uint32_t)((byte*)m_TextBatchEnds[batch] - (byte*)m_TextBatchStarts[batch]);
				if (dataSize)
				{
					uint32_t batchSize = 6 * ((dataSize / sizeof(TextVertex)) / 4);

					m_TextVertexBuffers[batch][frame]->SetData(m_TextBatchStarts[batch], dataSize);

					auto textResourceManager = m_TextRenderPass->GetResourceManager();
					textResourceManager->Update("u_MSDFSampler", m_Fonts[batch]->GetAtlasTexture());
					Z_CORE_ASSERT(textResourceManager->ReadyToRender());
					textResourceManager->ProcessChanges();

					Renderer::DrawIndexed(m_TextRenderPass, m_TextVertexBuffers[batch][frame], m_QuadIndexBuffer, batchSize);

					m_Stats.TextBatchCount++;
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

	void Renderer2D::MaybeAddNewTextBatch()
	{
		if (m_TextBatchStarts.size() >= m_Fonts.size())
			return;

		uint32_t framesInFlight = Renderer::GetFramesInFlight();

		auto& newVertexBufferPerFrame = m_TextVertexBuffers.emplace_back();
		newVertexBufferPerFrame.resize(framesInFlight);
		for (uint32_t frame = 0; frame < framesInFlight; frame++)
		{
			newVertexBufferPerFrame[frame] = VertexBuffer::Create(c_MaxQuadVerticesPerBatch * sizeof(TextVertex));
		}

		auto& newBatch = m_TextBatchStarts.emplace_back();
		newBatch = znew TextVertex[c_MaxQuadVerticesPerBatch];
		m_TextBatchEnds.emplace_back(newBatch);
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
			newVertex->TextureIndex = 0;
			newVertex->TilingFactor = 1.0f;
			newVertex->EntityID = entityID;

			newVertex++;
		}

		m_QuadIndexCount += 6;
		m_Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& tint, float tiling, int entityID)
	{
		Z_CORE_VERIFY(texture);

		m_LastQuadBatch = m_QuadIndexCount / c_MaxQuadIndicesPerBatch;

		if (m_LastQuadBatch >= m_QuadBatchStarts.size())
		{
			AddNewQuadBatch();
			m_QuadBatchEnds.emplace_back();
			m_QuadBatchEnds[m_LastQuadBatch] = m_QuadBatchStarts[m_LastQuadBatch];
		}

		uint32_t textureIndex = 0;
		{
			// check if texture is already in our array
			for (uint32_t i = 1; i < m_TextureSlotsInUse; i++)
			{
				if (m_TextureSlots[i]->GetAssetHandle() == texture->GetAssetHandle())
				{
					textureIndex = i;
					break;
				}
			}

			// otherwise, add it to the array, dynamically resizing if necessary/possible
			if (textureIndex == 0)
			{
				Z_CORE_ASSERT(m_TextureSlotsInUse < m_Specification.MaxTextureSlots, "Reached maximum bound textures");

				textureIndex = m_TextureSlotsInUse;
				m_TextureSlots[textureIndex] = texture;
				m_TextureSlotsInUse++;
			}
		}

		auto& newVertex = m_QuadBatchEnds[m_LastQuadBatch];
		for (int i = 0; i < 4; i++)
		{
			newVertex->Position = transform * m_QuadTemplate[i];
			newVertex->Tint = tint;
			newVertex->TextureCoord = m_TextureTemplate[i];
			newVertex->TextureIndex = textureIndex;
			newVertex->TilingFactor = tiling;
			newVertex->EntityID = entityID;

			newVertex++;
		}

		m_QuadIndexCount += 6;
		m_Stats.QuadCount++;
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
			newVertex->EntityID = entityID;

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
		newVertex->EntityID = entityID;
		newVertex++;
		
		newVertex->Position = end1;
		newVertex->Colour = colour;
		newVertex->EntityID = entityID;
		newVertex++;
		
		m_LineVertexCount += 2;
		m_Stats.LineCount++;
	}

	void Renderer2D::DrawQuadBoundingBox(const glm::mat4& transform, const glm::vec4& colour, int entityID, glm::vec3 rescale)
	{
		glm::mat4 rescaleTransform = glm::scale(glm::mat4(1.0f), rescale);

		for (int i = 0; i < 4; i++)
			DrawLine(transform * rescaleTransform * m_QuadTemplate[i], transform * rescaleTransform * m_QuadTemplate[(i + 1) % 4], colour, entityID);
	}

	void Renderer2D::DrawString(const glm::mat4 transform, const std::string& string, StringSpecification& spec, int entityID)
	{
		auto font = spec.Font;
		Z_CORE_VERIFY(font);
		
		auto fontGeometry = font->GetMSDFData()->FontGeometry;
		auto fontMetrics = fontGeometry.getMetrics();
		auto fontAssetHandle = font->GetAssetHandle();
		auto atlasTexture = font->GetAtlasTexture();

		uint32_t batch;
		
		// organising batches by font
		auto it = std::find_if(m_Fonts.rbegin(), m_Fonts.rend(), [fontAssetHandle](Ref<Font> f){ return f->GetAssetHandle() == fontAssetHandle; });
		if (it == m_Fonts.rend())
		{
			m_Fonts.emplace_back(font);
			m_Stats.FontCount++;
			MaybeAddNewTextBatch();
			batch = m_Fonts.size() - 1;
			m_Stats.TextBatchCount++;
		}
		else
		{
			batch = std::distance(std::begin(m_Fonts), it.base()) - 1;
		}

		glm::vec2 texelSize = { 1.0f / atlasTexture->GetWidth(), 1.0f / atlasTexture->GetHeight() };
		float fsScale = 1.0f / (fontMetrics.ascenderY - fontMetrics.descenderY);

		glm::vec4 cursor(.0f);

		for (uint32_t i = 0; i < string.length(); i++)
		{
			char character = string[i];

			auto glyph = fontGeometry.getGlyph(character);
			if (!glyph)
			{
				// TODO: check for control characters (mainly \n, \r, \t) and handle them (move cursor)
				continue;
			}

			// check for batch overflow
			if (m_TextBatchEnds[batch] - m_TextBatchStarts[batch] >= c_MaxQuadVerticesPerBatch)
			{
				m_Fonts.emplace_back(font);
				MaybeAddNewTextBatch();
				batch = m_Fonts.size() - 1;
				m_Stats.TextBatchCount++;
			}
			
			// compute uv bounds of character in atlas texture (u right, v down)
			double uMin, uMax, vMin, vMax;
			glyph->getQuadAtlasBounds(uMin, vMax, uMax, vMin);
			glm::vec2 atlasCoords[4] =
			{ 
				{ (float)uMin, (float)vMax },
				{ (float)uMax, (float)vMax },
				{ (float)uMax, (float)vMin },
				{ (float)uMin, (float)vMin }
			};
			for (int i = 0; i < 4; i++)
				atlasCoords[i] *= texelSize;

			// compute quad bounds in local coordinate plane (x right, y up)
			double xMin, xMax, yMin, yMax;
			glyph->getQuadPlaneBounds(xMin, yMin, xMax, yMax);
			glm::vec4 quadVertices[4] =
			{
				{ (float)xMin, (float)yMin, 0.f, 1.f },
				{ (float)xMax, (float)yMin, 0.f, 1.f },
				{ (float)xMax, (float)yMax, 0.f, 1.f },
				{ (float)xMin, (float)yMax, 0.f, 1.f }
			};
			for (int i = 0; i < 4; i++)
			{
				quadVertices[i] *= fsScale;
				quadVertices[i] += cursor;
			}

			// add character to batch
			auto& newVertex = m_TextBatchEnds[batch];
			for (int i = 0; i < 4; i++)
			{
				newVertex->Position = transform * quadVertices[i];
				newVertex->TextureCoord = atlasCoords[i];
				newVertex->EntityID = entityID;
				newVertex->FillColour = spec.FillColour;
				newVertex->BackgroundColour = spec.BackgroundColour;

				newVertex++;
			}

			// compute position of next character
			if (i < string.size() - 1)
			{
				double advance = glyph->getAdvance();
				char nextCharacter = string[i + 1];
				fontGeometry.getAdvance(advance, character, nextCharacter);

				// TODO: add support for kerning, custom offsets, multiple lines, word wrapping etc.
				cursor += glm::vec4(fsScale * advance, .0f, .0f, .0f);
			}

			m_Stats.CharCount++;
		}

		m_Stats.StringCount++;
	}

	void Renderer2D::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_QuadRenderPass->OnResize();
		m_CircleRenderPass->OnResize();
		m_LineRenderPass->OnResize();
		m_TextRenderPass->OnResize();
	}
}
