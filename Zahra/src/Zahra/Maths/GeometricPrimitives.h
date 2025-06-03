#pragma once

#include <glm/glm.hpp>

namespace Zahra
{
	/**
	 * @brief Specifies a triangle in Euclidean 3-space, via its vertices A,B,C.
	 */
	struct Triangle
	{
		glm::vec3 A, B, C;

		/**
		 * @brief Returns a normal vector for the triangle (zero if vertices are colinear).
		 */
		glm::vec3 Normal() const { return glm::cross(B - A, C - A); }
	};

	/**
	 * @brief Specifies an axis-aligned bounding box in Euclidean 3-space.
	 */
	struct AABB
	{
		glm::vec3 Vertex; /**< @brief The vertex minimising all three coordinates */
		glm::vec3 Dimensions; /**< @brief The extent of the box in all three dimensions */

		/**
		 * @brief Returns the box's central point.
		 */
		glm::vec3 Centre() { return Vertex + .5f * Dimensions; }
	};

	/**
	 * @brief Specifies an axis-aligned bounding box (rectangle) in Euclidean 2-space.
	 */
	struct AARect
	{
		glm::vec2 Vertex; /**< @brief The vertex minimising both coordinates */
		glm::vec2 Dimensions; /**< @brief The extent of the box in each dimension */

		/**
		 * @brief Returns the box's central point.
		 */
		glm::vec2 Centre() { return Vertex + .5f * Dimensions; }
	};

	/**
	 * @brief Specifies a ray (half-infinite line segment) in Euclidean 3-space.
	 */
	struct Ray
	{
		glm::vec3 Origin; /**< @brief The point from which the ray extends */

		glm::vec3 Direction; /**< @brief A vector parallel to the ray's direction */

		/**
		 * @brief Computes the possible intersection of this ray with a given triangle, returning whether/where this occurs.
		 * 
		 * Checks whether this ray will hit the front face of a triangle (as defined by the 
		 * right-hand rule, traversing the provided vertices counterclockwise, and assuming non-degeneracy).Returns whether such an
		 * intersection exists, and if so, feeds back where along the ray it is located.
		 * 
		 * @return True if such an intersection can be found, and False otherwise.
		 * 
		 * @param[in] T The triangle in question
		 * @param[out] s If an intersection is found, this will be set to the ray parameter at
		 which it occurs (i.e. Intersection = Origin + s * Direction).
		 */
		bool IntersectsTriangleFrontFace(const Triangle& T, float& s)
		{
			glm::vec3 n = T.Normal();
			float inverseSquare = 1.f / glm::dot(n, n);

			float nD = glm::dot(n, Direction);

			s = glm::dot(n, T.A - Origin) / nD;

			static constexpr float tolerance = .000001f;
			if (nD > -tolerance || s < tolerance) // TODO: these conditions may be too restrictive
				return false;
						
			glm::vec3 AX = Origin + s * Direction - T.A;

			// compute affine coordinates
			float u = inverseSquare * glm::dot(glm::cross(T.C - T.A, n), AX);
			float v = -inverseSquare * glm::dot(glm::cross(T.B - T.A, n), AX);
			// TODO: any performance difference compared to barycentric?

			return u >= 0 && v >= 0 && u + v <= 1;
		}
	};


}
