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
		alignas(16) glm::vec3 WorldPosition;
		alignas(16) glm::vec2 LocalPosition;
		alignas(16) glm::vec4 Colour;
		alignas(16) float Thickness;
		alignas(16) float Fade;
		alignas(16) int EntityID = -1;
	};

	struct LineVertex
	{
		alignas(16) glm::vec3 Position;
		alignas(16) glm::vec4 Colour;
		alignas(16) int EntityID = -1;
	};

}
