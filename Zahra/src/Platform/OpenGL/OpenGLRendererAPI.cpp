#include "zpch.h"
#include "OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace Zahra
{

	void Zahra::OpenGLRendererAPI::SetClearColour(const glm::vec4& colour)
	{
		glClearColor(colour.r, colour.g, colour.b, colour.a);
	}

	void Zahra::OpenGLRendererAPI::Clear()
	{
		// the specific flags should really be arguments to this function
		// (in any case, these two buffers should be cleared separately)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Zahra::OpenGLRendererAPI::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
	{
		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, NULL);
	}

}

