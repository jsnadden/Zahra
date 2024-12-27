#pragma once

namespace Zahra
{
	enum class PrimitiveTopology
	{
		Triangles,
		TriangleStrip,
		TriangleFan,
		Lines,
		LineStrip,
		Points
	};

	struct CameraData
	{
		glm::mat4 View = glm::mat4(1.0f);
		glm::mat4 Projection = glm::mat4(1.0f);
	};

}
