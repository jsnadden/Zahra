#pragma once

#include "RenderCommand.h"

#include "Camera.h"
#include "Shader.h"

namespace Zahra
{
	class Renderer
	{
	public:
		static void BeginScene(Camera& camera);
		static void EndScene();
		
		static void Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

	private:
		struct SceneData
		{
			glm::mat4 PVMatrix;
		};

		static SceneData* m_SceneData;
	};

};
