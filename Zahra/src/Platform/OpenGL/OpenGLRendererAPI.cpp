#include "zpch.h"
#include "OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace Zahra
{
	void OpenGLRendererAPI::Init()
	{
		Z_PROFILE_FUNCTION();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		Z_PROFILE_FUNCTION();

		glViewport(x, y, width, height);
	}

	void Zahra::OpenGLRendererAPI::SetClearColour(const glm::vec4& colour)
	{
		Z_PROFILE_FUNCTION();

		glClearColor(colour.r, colour.g, colour.b, colour.a);
	}

	void Zahra::OpenGLRendererAPI::Clear()
	{
		Z_PROFILE_FUNCTION();

		// TODO: the specific flags should really be arguments to this function
		// (in any case, these two buffers should be cleared separately)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Zahra::OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		Z_PROFILE_FUNCTION();
		
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();

		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

}

