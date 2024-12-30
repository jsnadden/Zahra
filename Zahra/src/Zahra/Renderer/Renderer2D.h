#pragma once

#include "Zahra/Core/Application.h"
#include "Zahra/Core/Types.h"
#include "Zahra/Renderer/Camera.h"
#include "Zahra/Renderer/EditorCamera.h"
#include "Zahra/Renderer/IndexBuffer.h"
#include "Zahra/Renderer/Mesh.h"
#include "Zahra/Renderer/RenderPass.h"
#include "Zahra/Renderer/Renderer.h"
#include "Zahra/Renderer/Shader.h"
#include "Zahra/Renderer/ShaderResourceManager.h"
#include "Zahra/Renderer/Texture.h"
#include "Zahra/Renderer/UniformBuffer.h"
#include "Zahra/Renderer/VertexBuffer.h"

namespace Zahra
{
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Tint;
		glm::vec2 TextureCoord;
		uint32_t TextureIndex;
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

	struct Renderer2DSpecification
	{
		Ref<Framebuffer> RenderTarget;
		uint32_t MaxBatchSize = 10000;
		uint32_t MaxTextureSlots = 32;
	};

	// TODO: add text rendering!

	class Renderer2D : public RefCounted
	{
	public:
		Renderer2D(Renderer2DSpecification specification);
		~Renderer2D();
		
		void BeginScene(const glm::mat4& cameraView, const glm::mat4& cameraProjection);
		//void BeginScene(const Camera& camera, const glm::mat4& transform);
		void BeginScene(const EditorCamera& camera);
		void EndScene();

		// TODO: Add billboarded options
		void DrawQuad(const glm::mat4& transform, const glm::vec4& colour, int entityID = -1);
		void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& tint = { 1.0f, 1.0f, 1.0f, 1.0f }, float tiling = 1.0f, int entityID = -1);
		void DrawCircle(const glm::mat4& transform, const glm::vec4& colour, float thickness, float fade, int entityID = -1);
		void DrawLine(const glm::vec3& end0, const glm::vec3& end1, const glm::vec4& colour, int entityID = -1);
		void DrawQuadBoundingBox(const glm::mat4& transform, const glm::vec4& colour, int entityID = -1);

		void SetLineWidth(float width) { m_LineWidth = width; }
		float GetLineWidth() { return m_LineWidth; }

		void OnViewportResize(uint32_t width, uint32_t height);

		struct Statistics
		{
			uint32_t QuadBatchCount = 0;
			uint32_t QuadCount = 0;

			uint32_t CircleBatchCount = 0;
			uint32_t CircleCount = 0;

			uint32_t LineBatchCount = 0;
			uint32_t LineCount = 0;

			uint32_t DrawCalls = 0;

			//uint32_t GetTotalVertexCount() { return 4 * QuadCount + 4 * CircleCount + ??? * LineCount; }
			//uint32_t GetTotalIndexCount() { return 6 * QuadCount + 6 * CircleCount + ??? * LineCount; }
		};

		const Statistics& GetStats() { return m_Stats; }

	private:
		Renderer2DSpecification m_Specification;

		const uint32_t c_MaxBatchSize = 10000;
		const uint32_t c_MaxQuadVerticesPerBatch = c_MaxBatchSize * 4;
		const uint32_t c_MaxQuadIndicesPerBatch = c_MaxBatchSize * 6;
		const uint32_t c_MaxLineVerticesPerBatch = c_MaxBatchSize * 2;

		float m_LineWidth = 1.5f;

		Statistics m_Stats;

		ShaderLibrary m_ShaderLibrary;

		glm::mat4 m_ProjectionView = glm::mat4(1.0f);
		// TODO: make this a push constant, not a UB
		Ref<UniformBufferSet> m_CameraUniformBufferSet;

		std::vector<Ref<Texture2D>> m_TextureSlots;
		uint32_t m_TextureSlotsInUse = 1; // start at 1, because slot 0 will be our default 1x1 white texture

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// QUADS
		glm::vec4 m_QuadTemplate[4]{};
		glm::vec2 m_TextureTemplate[4]{};

		//Ref<Shader> m_QuadShader;
		Ref<ShaderResourceManager> m_QuadResourceManager;
		Ref<RenderPass> m_QuadRenderPass;

		std::vector<std::vector<Ref<VertexBuffer>>>	m_QuadVertexBuffers; // indexed by (batch, frame)
		std::vector<QuadVertex*> m_QuadBatchStarts;
		std::vector<QuadVertex*> m_QuadBatchEnds;
		
		Ref<IndexBuffer> m_QuadIndexBuffer;

		uint32_t m_LastQuadBatch = 0;
		uint32_t m_QuadIndexCount = 0;

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CIRCLES
		//Ref<Shader>	m_CircleShader;
		Ref<ShaderResourceManager> m_CircleResourceManager;
		Ref<RenderPass> m_CircleRenderPass;

		std::vector<std::vector<Ref<VertexBuffer>>>	m_CircleVertexBuffers; // indexed by (batch, frame)
		std::vector<CircleVertex*> m_CircleBatchStarts;
		std::vector<CircleVertex*> m_CircleBatchEnds;

		uint32_t m_LastCircleBatch = 0;
		uint32_t m_CircleIndexCount = 0;

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// LINES
		//Ref<Shader> m_LineShader;
		Ref<ShaderResourceManager> m_LineResourceManager;
		Ref<RenderPass> m_LineRenderPass;

		std::vector<std::vector<Ref<VertexBuffer>>> m_LineVertexBuffers;
		std::vector<LineVertex*> m_LineBatchStarts;
		std::vector<LineVertex*> m_LineBatchEnds;

		uint32_t m_LastLineBatch = 0;
		uint32_t m_LineVertexCount = 0;

		void Init();
		void Shutdown();

		void ResetStats() { memset(&m_Stats, 0, sizeof(Statistics)); }

		void AddNewQuadBatch();
		void AddNewCircleBatch();
		void AddNewLineBatch();
	};
}
