#pragma once

#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>

namespace Zahra
{
	namespace Maths
	{
		/**
		 * @brief Attempts to decompose an E(3) (Euclidean group) element into TRS (Translation * Rotation * Scale) components.
		 * 
		 * @return True if decomposition is possible (within available numerical precision), False otherwise.
		 * 
		 * @param[in] transform The 4x4 matrix representing the given Euclidean group element (i.e. an affine transformation).
		 * @param[out] translation A vector in which to store the Translation component.
		 * @param[out] eulers A vector in which to store (a choice of) Euler angles representing the Rotation component.
		 * @param[out] scale A vector in which to store the eigenvalues of the Scale component.
		 */
		bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& eulers, glm::vec3& scale);
	}
}
