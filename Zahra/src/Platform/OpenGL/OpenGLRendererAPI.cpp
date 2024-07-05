#include "zpch.h"
#include "OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace Zahra
{

	void OpenGLMessageCallback(unsigned source, unsigned type, unsigned id, unsigned severity, int length, const char* message, const void* userParam)
	{
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:         Z_CORE_CRITICAL(message); return;
		case GL_DEBUG_SEVERITY_MEDIUM:       Z_CORE_ERROR(message); return;
		case GL_DEBUG_SEVERITY_LOW:          Z_CORE_WARN(message); return;
		case GL_DEBUG_SEVERITY_NOTIFICATION: Z_CORE_TRACE(message); return;
		}

		Z_CORE_ASSERT(false, "Unknown severity level!");
	}

	void OpenGLRendererAPI::Init()
	{
		Z_PROFILE_FUNCTION();

		#ifdef Z_DEBUG
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(OpenGLMessageCallback, nullptr);

			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
		#endif

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

