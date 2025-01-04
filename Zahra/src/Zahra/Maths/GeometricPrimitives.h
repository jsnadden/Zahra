#pragma once

#include <glm/glm.hpp>

namespace Zahra
{
	struct Triangle
	{
		glm::vec3 A, B, C;

		glm::vec3 Normal() const { return glm::cross(B - A, C - A); }
	};

	struct AABB
	{
		glm::vec3 Vertex, Dimensions;

		glm::vec3 Centre() { return Vertex + .5f * Dimensions; }
	};

	struct AARect
	{
		glm::vec2 Vertex, Dimensions;

		glm::vec2 Centre() { return Vertex + .5f * Dimensions; }
	};

	struct Ray
	{
		glm::vec3 Origin, Direction;

		bool IntersectsTriangleFrontFace(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, float& s)
		{
			glm::vec3 n = glm::cross(B - A, C - A);
			float inverseSquare = 1.f / glm::dot(n, n);

			float nD = glm::dot(n, Direction);

			s = glm::dot(n, A - Origin) / nD;

			static constexpr float tolerance = .000001f;
			if (nD > -tolerance || s < tolerance) // TODO: these conditions may be too restrictive
				return false;
						
			glm::vec3 AX = Origin + s * Direction - A;

			// compute affine coordinates
			float u = inverseSquare * glm::dot(glm::cross(C - A, n), AX);
			float v = -inverseSquare * glm::dot(glm::cross(B - A, n), AX);
			// TODO: any performance (dis)advantage compared to barycentric?

			return u >= 0 && v >= 0 && u + v <= 1;
		}
	};


}
