#include "zpch.h"
#include "Renderer.h"

#include "Renderer2D.h"


namespace Zahra
{

	Scope<Renderer::SceneData> Renderer::s_SceneData = CreateScope<Renderer::SceneData>();

	void Renderer::Init()
	{
		RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(Camera& camera)
	{
		s_SceneData->PVMatrix = camera.GetPVMatrix();
	}

	void Renderer::EndScene()
	{

	}

	void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		// TODO: instead this should stick a rendercommand into a queue
		//		also this needs to become renderapi agnostic eventually

		shader->Bind();
		shader->SetMat4("u_PVMatrix", s_SceneData->PVMatrix);
		shader->SetMat4("u_Transform", transform);
		

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
	

}
