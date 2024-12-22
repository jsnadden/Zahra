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

	// (these need to mirror the input layouts in their respective vertex shaders)

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Tint;
		glm::vec2 TextureCoord;

		// TODO: can I just pass these as push constants?
		//uint32_t TextureIndex;
		//float TilingFactor;
		//int EntityID = -1;
	};

	struct CircleVertex
	{
		glm::vec3 WorldPosition;
		glm::vec2 LocalPosition;
		glm::vec4 Colour;
		float Thickness;
		float Fade;
		//int EntityID = -1;
	};

	struct LineVertex
	{
		glm::vec3 Position;
		glm::vec4 Colour;
		//int EntityID = -1;
	};

	struct CameraData
	{
		glm::mat4 View = glm::mat4(1.0f);
		glm::mat4 Projection = glm::mat4(1.0f);
	};

}
