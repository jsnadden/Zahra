#include "zpch.h"
#include "Renderer.h"

namespace Zahra
{

	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

	void Renderer::BeginScene(Camera& camera)
	{
		m_SceneData->PVMatrix = camera.GetPVMatrix();
	}

	void Renderer::EndScene()
	{

	}

	void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		// TODO: instead this should stick a rendercommand into a queue

		shader->Bind();
		shader->UploadUniformMat4("u_PVMatrix", m_SceneData->PVMatrix);
		shader->UploadUniformMat4("u_Transform", transform);
		

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
	

}
