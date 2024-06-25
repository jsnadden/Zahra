#include "zpch.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Zahra
{

	Renderer3D::SceneData* Renderer3D::m_SceneData = new Renderer3D::SceneData;

	void Renderer3D::Init()
	{
		RenderCommand::Init();
	}

	void Renderer3D::BeginScene(Camera& camera)
	{
		m_SceneData->PVMatrix = camera.GetPVMatrix();
	}

	void Renderer3D::EndScene()
	{

	}

	void Renderer3D::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		// TODO: instead this should stick a rendercommand into a queue
		//		also this needs to become renderapi agnostic eventually

		shader->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_PVMatrix", m_SceneData->PVMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);
		

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
	

}
