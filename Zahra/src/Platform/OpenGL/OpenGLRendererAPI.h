#pragma once

#include "Zahra/Renderer/RendererAPI.h"

namespace Zahra
{

	class OpenGLRendererAPI : public RendererAPI
	{
	public:

		virtual void SetClearColour(const glm::vec4& colour) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;

	private:


	};

}
