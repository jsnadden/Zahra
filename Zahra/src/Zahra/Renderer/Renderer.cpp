#include "zpch.h"
#include "Renderer.h"

namespace Zahra
{

	///////////////////////////////////////////////////
	// TODO: IMPLEMENT THESE
	void Renderer::BeginScene()
	{

	}

	void Renderer::EndScene()
	{

	}
	//
	///////////////////////////////////////////////////

	void Renderer::Submit(const std::shared_ptr<VertexArray>& vertexArray)
	{
		// TODO: instead this should stick a rendercommand into a queue
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
	

}
